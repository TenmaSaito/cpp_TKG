#!/usr/bin/env python3
"""Send Japanese PR notifications via Discord webhook.

This script MUST run from trusted default-branch code in a notifications
workflow; it must never execute downloaded PR artifacts or PR code.
"""
import io
import json
import os
import sys
import urllib.error
import urllib.request
import zipfile

MAX_ARCHIVE_SIZE = 1_000_000
MAX_REPORT_SIZE = 100_000
MAX_MESSAGE_LENGTH = 1900  # Discord message content hard limit is 2000 chars.


def plain(value: object) -> str:
    """One safe-ish display line; mentions also disabled in webhook payload."""
    return str(value).replace("\r", " ").replace("\n", " ").replace("`", "′").strip()


def fetch_json(url: str) -> dict:
    request = urllib.request.Request(url, headers={
        "Accept": "application/vnd.github+json",
        "Authorization": f"Bearer {os.environ['GH_TOKEN']}",
        "X-GitHub-Api-Version": "2022-11-28",
    })
    with urllib.request.urlopen(request, timeout=20) as response:
        return json.load(response)


def report_for(run: dict) -> dict | None:
    repo = os.environ["GITHUB_REPOSITORY"]
    api = os.environ.get("GITHUB_API_URL", "https://api.github.com")
    info = fetch_json(f"{api}/repos/{repo}/actions/runs/{run['id']}/artifacts?per_page=100")
    matching = [
        artifact for artifact in info.get("artifacts", [])
        if artifact.get("name") == "pr-quality-report" and not artifact.get("expired")
    ]
    if not matching:
        return None

    request = urllib.request.Request(matching[0]["archive_download_url"], headers={
        "Authorization": f"Bearer {os.environ['GH_TOKEN']}",
        "Accept": "application/vnd.github+json",
        "X-GitHub-Api-Version": "2022-11-28",
    })
    with urllib.request.urlopen(request, timeout=30) as response:
        archive = response.read(MAX_ARCHIVE_SIZE + 1)
    if len(archive) > MAX_ARCHIVE_SIZE:
        raise ValueError("レポートが1MBの上限を超えました")

    with zipfile.ZipFile(io.BytesIO(archive)) as bundle:
        reports = []
        for name in ("file_names.json", "identifiers.json", "pr_metadata.json"):
            if name not in bundle.namelist():
                continue
            item = bundle.getinfo(name)
            if item.file_size > MAX_REPORT_SIZE:
                raise ValueError("展開レポートが100KBの上限を超えました")
            with bundle.open(item) as source:
                data = source.read(MAX_REPORT_SIZE + 1)
            if len(data) > MAX_REPORT_SIZE:
                raise ValueError("展開レポートが100KBの上限を超えました")
            reports.append(json.loads(data))
        return {"reports": reports}


def discord_post(message: str) -> None:
    url = os.environ.get("DISCORD_WEBHOOK_URL", "")
    if not url:
        print("DISCORD_WEBHOOK_URLが未設定のためDiscord通知を省略しました。")
        return

    # Split long reports into separate messages (not just truncate diagnostics).
    messages = []
    chunk = ""
    for original_line in message.splitlines():
        line = original_line[:MAX_MESSAGE_LENGTH]
        proposed = chunk + ("\n" if chunk else "") + line
        if len(proposed) > MAX_MESSAGE_LENGTH and chunk:
            messages.append(chunk)
            chunk = line
        else:
            chunk = proposed
    if chunk:
        messages.append(chunk)

    for part in messages:
        payload = json.dumps({
            "username": "PR Review Bot",
            "content": part,
            "allowed_mentions": {"parse": []},
        }, ensure_ascii=False).encode("utf-8")
        request = urllib.request.Request(
            url,
            data=payload,
            headers={"Content-Type": "application/json", "User-Agent": "PRGuard/1.0"},
            method="POST",
        )
        with urllib.request.urlopen(request, timeout=15) as response:
            print("Discord HTTP status:", response.status)


def completed_message(run: dict) -> str:
    result = run.get("conclusion") or "unknown"
    status = {
        "success": "成功", "failure": "失敗", "cancelled": "キャンセル", "skipped": "スキップ",
        "timed_out": "タイムアウト", "action_required": "要対応",
    }.get(result, plain(result))
    prs = run.get("pull_requests") or []
    pr_number = prs[0].get("number") if prs else None
    pr_desc = f"PR #{pr_number}" if pr_number else "PR"
    icon = "✅" if result == "success" else "⚠️"
    lines = [f"{icon} **{pr_desc} 自動検査完了：{status}**", f"[GitHub Actionsの結果]({run['html_url']})"]
    try:
        report = report_for(run)
        if report is None:
            lines.append("詳細レポートなし（ワークフローの失敗など）")
            return "\n".join(lines)
        issues_total = 0
        for detail in report.get("reports", []):
            if "pr_number" in detail:
                if pr_number is None and isinstance(detail.get("pr_number"), int):
                    lines[0] = f"{icon} **PR #{detail['pr_number']} 自動検査完了：{status}**"
                continue
            items = detail.get("issues", [])
            issues_total += len(items)
            label = {"filename": "ファイル名", "identifiers": "C++識別子"}.get(
                detail.get("check"), "その他の検査"
            )
            lines.append(f"**{label}：{len(items)}件**")
            for item in items[:12]:
                file = plain(item.get("file", "?"))
                lineno = plain(item.get("line", 1))
                reason = plain(item.get("rule", item.get("message", "")))
                suggestion = plain(item.get("suggestion", "?"))
                lines.append(f"• `{file}:{lineno}` {reason} → `{suggestion}`")
            if len(items) > 12:
                lines.append("残りはGitHub Actionsのレポートを参照")
        if issues_total == 0:
            lines.append("命名規則違反は検出されませんでした。")
    except Exception as error:
        print("レポート取得に失敗しました:", error)
        lines.append("詳細レポートの取得に失敗しました。")
    return "\n".join(lines)


def main() -> None:
    if len(sys.argv) != 2 or sys.argv[1] not in ("opened", "completed"):
        raise SystemExit("使い方: notify_discord.py opened|completed")
    with open(os.environ["GITHUB_EVENT_PATH"], encoding="utf-8") as event_file:
        event = json.load(event_file)

    if sys.argv[1] == "opened":
        pr = event["pull_request"]
        msg = (
            f"🔔 **PR #{pr['number']} が作成・再オープンされました**\n"
            f"タイトル：{plain(pr['title'])}\n"
            f"[PRを開く]({pr['html_url']})"
        )
    else:
        msg = completed_message(event["workflow_run"])
    discord_post(msg)


if __name__ == "__main__":
    main()
