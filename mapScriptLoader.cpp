//==================================================================================
// 
// マップスクリプト読み込みクラスのヘッダーファイル [mapScriptLoader.cpp]
// Author : TENMA SAITO
// 
//==================================================================================
//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "mapScriptLoader.h"
#include "filestream.h"
#include "texture.h"
#include "player.h"
#include "item.h"
#include "field.h"
#include "wall.h"
#include "playerCamera.h"

//==================================================================================
// --- コンストラクタ ---
//==================================================================================
CMapScriptLoader::CMapScriptLoader()
{
}

//==================================================================================
// --- デストラクタ ---
//==================================================================================
CMapScriptLoader::~CMapScriptLoader()
{
}

//==================================================================================
// --- スクリプト読み込み処理 ---
//==================================================================================
HRESULT CMapScriptLoader::Load(std::string_view path)
{
	HRESULT hr = S_OK;		// 読み込み結果
	std::string line;		// 読み込んだ文字列
	CTexture *pTexture = CTexture::GetInstance();			// テクスチャへのポインタ
	std::unique_ptr pFile = std::make_unique<CFileStream>();		// ファイルストリームへのポインタ

	if (pFile->Open(path, false) == false)
	{ // ファイルオープン失敗時
		hr = E_FAIL;
		return hr;
	}

	while (1)
	{ // SCRIPTキーワードを検索
		// 文字列を一列読み込み
		pFile->ReadString(line);
		if (pFile->FindString(line, "SCRIPT") == true)
		{ // 見つかった場合
			break;
		}
		else if (pFile->IsEoF() == true)
		{ // 見つからなかった場合
			hr = S_FALSE;
			return hr;
		}
	}

	while (1)
	{ // 各オブジェクトを配置
		// 文字列を一列読み込み
		pFile->ReadString(line);
		if (pFile->FindString(line, "END_SCRIPT") || pFile->IsEoF())
		{ // 読み込み終了
			break;
		}
		else if (pFile->FindString(line, "PLAYERSET"))
		{ // プレイヤーの設置キーワードの場合
			std::string sModelPath;			// モデルのパス
			Vector3 pos = VECTOR3_NULL;		// 位置
			float fRadius = 0.0f;			// 当たり判定の半径

			while (1)
			{ // プレイヤーの設置に必要な各情報を読み込み
				size_t find = 0U;		// 見つけた文字列の位置
				std::string_view after;	// コメント・タブスペース消去後の文字列

				// 文字列を一列読み込み
				pFile->ReadString(line);
				after = line;		// 文字列の参照を取得

				// コメント消去
				auto comment = line.find_last_of('#');
				if (comment != std::string_view::npos)
				{ // コメントが見つかった場合、そこを切り捨て
					line = line.substr(0U, comment);
				}

				while (1)
				{// タブスペース消去
					auto tab = std::ranges::find(line, '\t');
					if (tab != line.end())
					{ // タブスペースが見つかった場合、そこを切り捨て
						line.erase(tab);
					}
					else
					{ // 見つからなくなったら終了
						break;
					}
				}

				if (pFile->FindString(line, "END_PLAYERSET"))
				{ // プレイヤーの情報が終了した場合、設置
					CPlayer *pPlayer = CPlayer::Create(sModelPath.c_str(), pos, fRadius);

					// プレイヤーカメラも設置
					CPlayerCamera::Create(pPlayer);
					break;
				}
				else if (pFile->FindString(line, "POS = ", &find, true))
				{ // 位置の情報だった場合、読み込み
					pos = pFile->ToVector3(&line[find]);
				}
				else if (pFile->FindString(line, "RADIUS = ", &find, true))
				{ // 位置の情報だった場合、読み込み
					fRadius = pFile->ToFloat(&line[find]);
				}
				else if (pFile->FindString(line, "MODEL_FILENAME = ", &find, true))
				{ // モデルパス情報だった場合、コメント消去後読み込み
					std::string_view filename = &line[find];		// 見つかった文字列
					sModelPath = filename;		// モデルのパスを保存
				}
			}
		}
		else if (pFile->FindString(line, "ITEMSET"))
		{ // アイテムの設置キーワードの場合
			std::string sModelPath;			// モデルのパス
			Vector3 pos = VECTOR3_NULL;		// 位置
			float fRadius = 0.0f;			// 半径

			while (1)
			{ // アイテムの設置に必要な各情報を読み込み
				size_t find = 0U;		// 見つけた文字列の位置
				std::string_view after;	// コメント・タブスペース消去後の文字列

				// 文字列を一列読み込み
				pFile->ReadString(line);
				after = line;		// 文字列の参照を取得

				// コメント消去
				auto comment = line.find_last_of('#');
				if (comment != std::string_view::npos)
				{ // コメントが見つかった場合、そこを切り捨て
					line = line.substr(0U, comment);
				}

				while (1)
				{// タブスペース消去
					auto tab = std::ranges::find(line, '\t');
					if (tab != line.end())
					{ // タブスペースが見つかった場合、そこを切り捨て
						line.erase(tab);
					}
					else
					{ // 見つからなくなったら終了
						break;
					}
				}

				if (pFile->FindString(line, "END_ITEMSET"))
				{ // アイテムの情報が終了した場合、設置
					CItem::Create(sModelPath, pos, fRadius);
					break;
				}
				else if (pFile->FindString(line, "POS = ", &find, true))
				{ // 位置の情報だった場合、読み込み
					pos = pFile->ToVector3(&line[find]);
				}
				else if (pFile->FindString(line, "RADIUS = ", &find, true))
				{ // 位置の情報だった場合、読み込み
					fRadius = pFile->ToFloat(&line[find]);
				}
				else if (pFile->FindString(line, "MODEL_FILENAME = ", &find, true))
				{ // モデルパス情報だった場合、コメント消去後読み込み
					std::string_view filename = &line[find];		// 見つかった文字列
					sModelPath = filename;		// モデルのパスを保存
				}
			}
		}
		else if (pFile->FindString(line, "FIELDSET"))
		{ // 床の設置キーワードの場合
			std::string sTexturePath;		// テクスチャのパス
			Vector3 pos = VECTOR3_NULL;		// 位置
			Vector2 size = VECTOR2_NULL;	// サイズ

			while (1)
			{ // 床の設置に必要な各情報を読み込み
				size_t find = 0U;		// 見つけた文字列の位置
				std::string_view after;	// コメント・タブスペース消去後の文字列

				// 文字列を一列読み込み
				pFile->ReadString(line);
				after = line;		// 文字列の参照を取得

				// コメント消去
				auto comment = line.find_last_of('#');
				if (comment != std::string_view::npos)
				{ // コメントが見つかった場合、そこを切り捨て
					line = line.substr(0U, comment);
				}

				while (1)
				{// タブスペース消去
					auto tab = std::ranges::find(line, '\t');
					if (tab != line.end())
					{ // タブスペースが見つかった場合、そこを切り捨て
						line.erase(tab);
					}
					else
					{ // 見つからなくなったら終了
						break;
					}
				}

				if (pFile->FindString(line, "END_FIELDSET"))
				{ // 床の情報が終了した場合、設置
					CField *pField = CField::Create(pos, VECTOR3_NULL, size);
					pField->BindTexture(pTexture->Register(sTexturePath));
					break;
				}
				else if (pFile->FindString(line, "POS = ", &find, true))
				{ // 位置の情報だった場合、読み込み
					pos = pFile->ToVector3(&line[find]);
				}
				else if (pFile->FindString(line, "SIZE = ", &find, true))
				{ // サイズの情報だった場合、読み込み
					size = pFile->ToVector2(&line[find]);
				}
				else if (pFile->FindString(line, "TEXTURE_FILENAME = ", &find, true))
				{ // テクスチャパス情報だった場合、コメント消去後読み込み
					std::string_view filename = &line[find];		// 見つかった文字列
					sTexturePath = filename;		// テクスチャのパスを保存
				}
			}
		}
		else if (pFile->FindString(line, "WALLSET"))
		{ // 壁の設置キーワードの場合
			std::string sTexturePath;		// テクスチャのパス
			Vector3 pos = VECTOR3_NULL;		// 位置
			Vector3 rot = VECTOR3_NULL;		// 角度
			Vector2 size = VECTOR2_NULL;	// サイズ

			while (1)
			{ // 壁の設置に必要な各情報を読み込み
				size_t find = 0U;		// 見つけた文字列の位置
				std::string_view after;	// コメント・タブスペース消去後の文字列

				// 文字列を一列読み込み
				pFile->ReadString(line);
				after = line;		// 文字列の参照を取得

				// コメント消去
				auto comment = line.find_last_of('#');
				if (comment != std::string_view::npos)
				{ // コメントが見つかった場合、そこを切り捨て
					line = line.substr(0U, comment);
				}

				while (1)
				{// タブスペース消去
					auto tab = std::ranges::find(line, '\t');
					if (tab != line.end())
					{ // タブスペースが見つかった場合、そこを切り捨て
						line.erase(tab);
					}
					else
					{ // 見つからなくなったら終了
						break;
					}
				}

				if (pFile->FindString(line, "END_WALLSET"))
				{ // 壁の情報が終了した場合、設置
					CWall *pWall = CWall::Create(pos, rot, size);
					pWall->BindTexture(pTexture->Register(sTexturePath));
					break;
				}
				else if (pFile->FindString(line, "POS = ", &find, true))
				{ // 位置の情報だった場合、読み込み
					pos = pFile->ToVector3(&line[find]);
				}
				else if (pFile->FindString(line, "ROT = ", &find, true))
				{ // 位置の情報だった場合、読み込み
					rot = pFile->ToVector3(&line[find]);
				}
				else if (pFile->FindString(line, "SIZE = ", &find, true))
				{ // サイズの情報だった場合、読み込み
					size = pFile->ToVector2(&line[find]);
				}
				else if (pFile->FindString(line, "TEXTURE_FILENAME = ", &find, true))
				{ // テクスチャパス情報だった場合、コメント消去後読み込み
					std::string_view filename = &line[find];		// 見つかった文字列
					sTexturePath = filename;		// テクスチャのパスを保存
				}
			}
		}
	}

	return hr;
}