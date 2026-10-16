//==================================================================================
// 
// マネージャクラスのソースファイル [manager.cpp]
// Author : TENMA SAITO
// Date   : 2026/5/12
// 
//==================================================================================
//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "manager.h"
#include "renderer.h"
#include "input.h"
#include "joypad.h"
#include "debugproc.h"
#include "sound.h"
#include "object.h"
#include "camera.h"
#include "playerCamera.h"
#include "light.h"
#include "texture.h"
#include "xfile.h"
#include "util.h"
#include "matrix.h"
#include "mapScriptLoader.h"
#include "player.h"
#include "timer.h"
#include "vec2math.h"
#include "vec3math.h"

//==================================================================================
// --- マネージャの取得処理 ---
//==================================================================================
CManager *CManager::GetInstance(void)
{
	static CManager manager;		// インスタンス
	return &manager;
}

//==================================================================================
// --- コンストラクタ ---
//==================================================================================
CManager::CManager()
{
}

//==================================================================================
// --- デストラクタ ---
//==================================================================================
CManager::~CManager()
{
}

//==================================================================================
// --- 初期化処理 ---
//==================================================================================
HRESULT CManager::Init(const HINSTANCE hInstance, const HWND hWnd, const BOOL bWindow)
{
	HRESULT hr = S_OK;		// 各関数の結果

	if (hWnd == nullptr)
	{ // ウィンドウハンドルがNULLの場合、失敗
		return E_FAIL;
	}

	// ウィンドウハンドルを保存
	m_hWnd = hWnd;

	// レンダラーの生成
	if (m_pRenderer == nullptr)
	{ // レンダラーがNULLの場合
		// レンダラーを生成
		m_pRenderer = std::make_unique<CRenderer>();
		if (m_pRenderer == nullptr)
		{ // 生成失敗
			MessageBox(hWnd, "レンダラーの生成に失敗しました！", "Failed", MB_ICONERROR);
			return E_FAIL;
		}

		// レンダラーの初期化
		hr = m_pRenderer->Init(hWnd, bWindow);
		if (FAILED(hr))
		{ // レンダラーの初期化失敗
			MessageBox(hWnd, "レンダラーの初期化に失敗しました！", "Failed", MB_ICONERROR);
			return E_FAIL;
		}
	}

	// キーボードの生成
	if (m_pInputKeyboard == nullptr)
	{ // もしまだ生成されていないなら
		// キーボードを生成
		m_pInputKeyboard = std::make_unique<CInputKeyboard>();
		if (m_pInputKeyboard == nullptr)
		{ // 生成失敗
			MessageBox(hWnd, "キーボードの生成に失敗しました！", "Failed", MB_ICONERROR);
			return E_FAIL;
		}

		// キーボードの初期化
		hr = m_pInputKeyboard->Init(hInstance, hWnd);
		if (FAILED(hr))
		{ // キーボードの初期化失敗
			MessageBox(hWnd, "キーボードの初期化に失敗しました！", "Failed", MB_ICONERROR);
			return E_FAIL;
		}
	}

	// マウスの生成
	if (m_pInputMouse == nullptr)
	{ // もしまだ生成されていないなら
		// マウスを生成
		m_pInputMouse = std::make_unique<CInputMouse>();
		if (m_pInputMouse == nullptr)
		{ // 生成失敗
			MessageBox(hWnd, "マウスの生成に失敗しました！", "Failed", MB_ICONERROR);
			return E_FAIL;
		}

		// マウスの初期化
		hr = m_pInputMouse->Init(hInstance, hWnd);
		if (FAILED(hr))
		{ // マウスの初期化失敗
			MessageBox(hWnd, "マウスの初期化に失敗しました！", "Failed", MB_ICONERROR);
			return E_FAIL;
		}
	}

	// ジョイパッドの生成
	if (m_pJoypad == nullptr)
	{ // もしまだ生成されていないなら
		// ジョイパッドを生成
		m_pJoypad = std::make_unique<CJoypad>();
		if (m_pJoypad == nullptr)
		{ // 生成失敗
			MessageBox(hWnd, "ジョイパッドの生成に失敗しました！", "Failed", MB_ICONERROR);
			return E_FAIL;
		}

		// ジョイパッドの初期化
		m_pJoypad->Init();
	}

	// デバッグ表示の生成
	if (m_pDebugProc == nullptr)
	{ // もしまだ生成されていないなら
		// デバッグ表示を生成
		m_pDebugProc = std::make_unique<CDebugProc>();
		if (m_pDebugProc == nullptr)
		{ // 生成失敗
			MessageBox(hWnd, "デバッグ表示の生成に失敗しました！", "Failed", MB_ICONERROR);
			return E_FAIL;
		}

		// デバッグ表示の初期化
		hr = m_pDebugProc->Init(DEFAULT_SIZE, "PixelMplus12");
		if (FAILED(hr))
		{ // デバッグ表示の初期化失敗
			MessageBox(hWnd, "デバッグ表示の初期化に失敗しました！", "Failed", MB_ICONERROR);
			return E_FAIL;
		}
	}

	// 音声の生成
	if (m_pSound == nullptr)
	{ // もしまだ生成されていないなら
		// サウンドを生成
		m_pSound = std::make_unique<CSound>();
		if (m_pSound == nullptr)
		{ // 生成失敗
			MessageBox(hWnd, "サウンドの生成に失敗しました！", "Failed", MB_ICONERROR);
			return E_FAIL;
		}

		// サウンドの初期化
		hr = m_pSound->Init(hWnd);
		if (FAILED(hr))
		{ // デバッグ表示の初期化失敗
			MessageBox(hWnd, "サウンドの初期化に失敗しました！", "Failed", MB_ICONERROR);
			return E_FAIL;
		}
	}

	// テクスチャの読み込み
	CTexture::GetInstance()->Load();

	// ライトの生成
	m_pLight = std::make_unique<CLight>();
	m_pLight->Init();

	CTimer *pTimer = CTimer::Create(Vector3(640.0f - 75.0f, 50.0f, 0.0f), Vector2(150.0f, 200.0f), 3);
	pTimer->SetTimer(100);
	pTimer->SetUpdate(true);

#pragma region Objects Create
	// マップ読み込み
	std::unique_ptr pMapScriptLoader = std::make_unique<CMapScriptLoader>();		// マップ読み込みインスタンス
	pMapScriptLoader->Load("data/SCRIPT/system.ini");
#pragma endregion

	// 成功
	return S_OK;
}

//==================================================================================
// --- 終了処理 ---
//==================================================================================
void CManager::Uninit(void)
{
	// 全オブジェクトの破棄, 終了処理
	CObject::ReleaseAll();

	// 全カメラの破棄、終了処理
	CCamera::ReleaseAll();

	// XFILEの破棄
	CXFile::GetInstance()->Unload();

	// テクスチャの破棄
	CTexture::GetInstance()->Unload();

	// ライトオブジェクトの破棄
	SafeUniqueUninit(m_pLight);

	// サウンドオブジェクトの破棄
	SafeUniqueUninit(m_pSound);

	// デバッグ表示オブジェクトの破棄
	SafeUniqueUninit(m_pDebugProc);
	
	// ジョイパッドオブジェクトの破棄
	SafeUniqueUninit(m_pJoypad);

	// マウスオブジェクトの破棄
	SafeUniqueUninit(m_pInputMouse);

	// キーボードオブジェクトの破棄
	SafeUniqueUninit(m_pInputKeyboard);

	// レンダラーオブジェクトの破棄
	SafeUniqueUninit(m_pRenderer);
}

//==================================================================================
// --- 更新処理 ---
//==================================================================================
void CManager::Update(void)
{
	// キーボードの更新処理
	m_pInputKeyboard->Update();

	// マウスの更新処理
	m_pInputMouse->Update();

	// ジョイパッドの更新処理
	m_pJoypad->Update();

	// デバッグ表示の更新処理
	m_pDebugProc->Update();
	
#ifdef _DEBUG
	// デバッグ情報
	m_pDebugProc->Print("======== デバッグ情報 ========\n");
	m_pDebugProc->Print("FPS : ({})\n", m_nCountFPS);
	m_pDebugProc->Print("リロード : ({}) [BackSpace]\n\n", m_nCountReload);
#endif
	// 操作説明
	m_pDebugProc->ForcePrint("======== 操作 ========\n");
	m_pDebugProc->ForcePrint("WASD : 台を傾ける\n");
	m_pDebugProc->ForcePrint("BACK : リセット\n");

	// ライトの更新処理
	m_pLight->Update();

	// カメラの更新
	CCamera::UpdateAll();

	// ポーズ
	if (m_pInputKeyboard->GetTrigger(DIK_P)) m_bPause ^= m_bPause;

	if (m_pInputKeyboard->GetTrigger(DIK_BACK))
	{ // リセット
		CObject::ReleaseAll();
		CCamera::ReleaseAll();
		m_rot = VECTOR3_NULL;

		// マップの再読み込み
		std::unique_ptr pMapScriptLoader = std::make_unique<CMapScriptLoader>();		// マップ読み込みインスタンス
		pMapScriptLoader->Load("data/SCRIPT/system.ini");
		m_nCountReload++;

		// タイマー設置
		CTimer *pTimer = CTimer::Create(Vector3(640.0f - 75.0f, 50.0f, 0.0f), Vector2(150.0f, 200.0f), 3);
		pTimer->SetTimer(100);
		pTimer->SetUpdate(true);
	}

	if (m_bPause == false)
	{ // レンダラーの更新
		m_pRenderer->Update();
	}

	// フレームカウンターを増加
	m_nCounterFrame++;
}

//==================================================================================
// --- 描画処理 ---
//==================================================================================
void CManager::Draw(void)
{ // レンダラーの描画
	Mtx::Identity(&m_mtxWorld);

	Mtx::CalcRotation(&m_mtxWorld, m_rot * QUARTER_PI * 0.5f);

	m_pRenderer->Draw();
}

//==================================================================================
// --- ポーズ状態設定処理 ---
//==================================================================================
void CManager::SetEnablePause(const bool bEnable)
{
	m_bPause = bEnable;
}

//==================================================================================
// --- ポーズ状態取得処理 ---
//==================================================================================
bool CManager::GetEnablePause(void)
{
	return false;
}