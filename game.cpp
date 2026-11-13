//==================================================================================
// 
// ゲームシーンのソースファイル [game.cpp]
// Author : TENMA SAITO
// Date   : 2026/9/28
// 
//==================================================================================
//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "game.h"
#include "mapScriptLoader.h"
#include "itemGenerator.h"
#include "screenEffect.h"
#include "rankingClient.h"
#include "score.h"
#include "sound.h"
#include "texture.h"
#include "manager.h"
#include "input.h"
#include "joypad.h"
#include "nextInfo.h"
#include "timer.h"
#include "foodBackground.h"
#include "filestream.h"

//**********************************************************************************
// *** 定数宣言 ***
//**********************************************************************************
namespace
{
	constexpr std::string_view SERVER_IPADDRESS = "127.0.0.1";		// ランキングサーバーのIPアドレス
	constexpr u_short SERVER_PORT = 22333;							// ランキングサーバーのポート番号
	constexpr const char *NUMBER_TEXTURE = "data/TEXTURE/voltNumber003.png";	// 数値のテクスチャ
}
	
//==================================================================================
// --- コンストラクタ ---
//==================================================================================
CGame::CGame()
{
}

//==================================================================================
// --- デストラクタ ---
//==================================================================================
CGame::~CGame()
{
}

//==================================================================================
// --- 初期化処理 ---
//==================================================================================
HRESULT CGame::Init(void)
{ // 開始処理
	Start();

	return S_OK;
}

//==================================================================================
// --- 終了処理 ---
//==================================================================================
void CGame::Uninit(void)
{
}

//==================================================================================
// --- 更新処理 ---
//==================================================================================
void CGame::Update(void)
{
	CManager *pManager = CManager::GetInstance();
	CInputKeyboard *pKeyboard = pManager->GetInputKeyboard();
	CJoypad *pJoypad = pManager->GetJoypad();

	if ((pKeyboard->GetTrigger(DIK_P)
		|| pJoypad->GetTrigger(CJoypad::KEY_START))
		&& pManager->IsTransition() == false)
	{
		pManager->SetEnablePause(!pManager->GetEnablePause());
	}

	if (CObject::FindAnyObjectByType<CTimer>(DEFAULT_TIMER_PRIORITY)->GetTimer() <= 0
		&& CManager::GetInstance()->IsTransition() == false)
	{
		auto pRankingClient = std::make_unique<CRankingClient>();
		auto pScore = CObject::FindAnyObjectByType<CScore>();
		if (SUCCEEDED(pRankingClient->Init(SERVER_IPADDRESS, SERVER_PORT)))
		{ // 初期化成功時のみ送信
			pRankingClient->Send(pScore->GetScore());
			pRankingClient->Uninit();
		}

		auto pFile = std::make_unique<CFileStream>("data/bin/current.bin", true, CFileStream::FLAG_OVERWRITE);
		if (pFile->IsOpen())
		{ // ファイルに書き出し
			pFile->Write(pScore->GetScore());
			pFile->Close();
		}

		// 画面を遷移させる
		CManager::GetInstance()->SetTransition(MODE_RANKING);
	}
}

//==================================================================================
// --- 描画処理 ---
//==================================================================================
void CGame::Draw(void)
{
}

//==================================================================================
// --- 開始時処理 ---
//==================================================================================
void CGame::Start(void)
{ // マップ読み込み
	std::unique_ptr pMapScriptLoader = std::make_unique<CMapScriptLoader>();		// マップ読み込みインスタンス
	auto hr = pMapScriptLoader->Load("data/SCRIPT/system.ini");

	// アイテム生成器起動
	(new CItemGenerator)->Init();

	// バックの食べ物出現
	CFoodBackground::Create();

	// スクリーンエフェクト生成
	CScreenEffect::Create(Color(1.0f, 0.0f, 0.0f, 0.0f), 60.0f);

	CScore::Create(CTexture::GetInstance()->Register("data/TEXTURE/voltNumber003.png"),
		Vector3(SCREEN_MIDDLE.x * 1.35f, 30.0f, 0.0f),
		Vector2(350.0f, 70.0f),
		8);

	CNextInfo::Create();

	CManager::GetInstance()->GetSound()->Play(CSound::LABEL_BGM_INGAME);
}