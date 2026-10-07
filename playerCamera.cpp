//==================================================================================
// 
// プレイヤーカメラクラスのソースファイル [playerCamera.cpp]
// Author : TENMA SAITO
// 
//==================================================================================
//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "playerCamera.h"
#include "player.h"
#include "manager.h"
#include "input.h"
#include "debugproc.h"
#include "vec3math.h"

//**********************************************************************************
// *** 定数宣言 ***
//**********************************************************************************
namespace
{
	constexpr float LENGTH_PLAYER = 400.0f;		// カメラとプレイヤーの距離
	constexpr float ROTATE_SPD = 0.04f;			// カメラの回転速度
}

//==================================================================================
// --- プレイヤーカメラ生成 ---
//==================================================================================
CPlayerCamera *CPlayerCamera::Create(const CPlayer *pPlayer)
{
	CPlayerCamera *pPlayerCam = static_cast<CPlayerCamera*>(GetCamera(TYPE_PLAYER));		// プレイヤーカメラを取得
	if (pPlayerCam == nullptr)
	{ // nullで生成されていなければ、生成
		pPlayerCam = new CPlayerCamera;
	}

	// 生成出来ていれば各処理を実行
	if (pPlayerCam != nullptr)
	{ // 初期化
		pPlayerCam->Init(pPlayer);

		// フォーカスを設定
		pPlayerCam->SetFocus();
	}

	return pPlayerCam;
}

//==================================================================================
// --- コンストラクタ ---
//==================================================================================
CPlayerCamera::CPlayerCamera() : CCamera(TYPE_PLAYER)
{
}

//==================================================================================
// --- デストラクタ ---
//==================================================================================
CPlayerCamera::~CPlayerCamera()
{
}

//==================================================================================
// --- 初期化処理 ---
//==================================================================================
void CPlayerCamera::Init(const CPlayer *pPlayer)
{ // 引数を保存
	m_pPlayer = pPlayer;

	Vector3 posV = *CCamera::GetPosV();		// 視点座標
	Vector3 posR = *CCamera::GetPosR();		// 注視点座標
	Vector3 rot = *CCamera::GetRotate();	// 角度

	// 角度を調整
	rot.z = -1.16f;

	// プレイヤーの座標を注視点に設定
	posR = *m_pPlayer->GetPosition();

	// 視点座標を注視点座標から求める
	posV.x = posR.x + (sinf(rot.z) * sinf(rot.y) * LENGTH_PLAYER);
	posV.y = posR.y + (cosf(rot.z) * LENGTH_PLAYER);
	posV.z = posR.z + (sinf(rot.z) * cosf(rot.y) * LENGTH_PLAYER);

	// 位置と角度を適用
	CCamera::Init(posV, posR);
	CCamera::SetRotate(rot);
}

//==================================================================================
// --- 終了処理 ---
//==================================================================================
void CPlayerCamera::Uninit(void)
{ // カメラの解放
	CCamera::Release();
}

//==================================================================================
// --- 更新処理 ---
//==================================================================================
void CPlayerCamera::Update(void)
{
	Vector3 posV = *CCamera::GetPosV();		// 視点座標
	Vector3 posR = *CCamera::GetPosR();		// 注視点座標
	Vector3 rot = *CCamera::GetRotate();	// 角度
	CManager *pManager = CManager::GetInstance();		// マネージャへのポインタ
	CInputKeyboard *pKeyboard = pManager->GetInputKeyboard();		// キーボードへのポインタ

	// 視点回転
	if (pKeyboard->GetPress(DIK_Q))
	{
		rot.y -= ROTATE_SPD;
	}
	else if (pKeyboard->GetPress(DIK_E))
	{
		rot.y += ROTATE_SPD;
	}

	// プレイヤーの座標を注視点に設定
	posR = *m_pPlayer->GetPosition();

	// 視点座標を注視点座標から求める
	posV.x = posR.x + (sinf(rot.z) * sinf(rot.y) * LENGTH_PLAYER);
	posV.y = posR.y + (cosf(rot.z) * LENGTH_PLAYER);
	posV.z = posR.z + (sinf(rot.z) * cosf(rot.y) * LENGTH_PLAYER);

	// 位置と角度を適用
	CCamera::SetPosV(posV);
	CCamera::SetPosR(posR);
	CCamera::SetRotate(Vec3::FixedRotation(rot));

	// プレイヤーの位置を表示
	CDebugProc *pProc = CManager::GetInstance()->GetDebugProc();		// デバッグ表示へのポインタ
	pProc->Print("====== カメラ ======\n");
	pProc->Print("視点   ({:.2f}/{:.2f}/{:.2f}) [Q/E]\n", PRINT_VECTOR3(*GetPosV()));
	pProc->Print("注視点 ({:.2f}/{:.2f}/{:.2f})\n", PRINT_VECTOR3(*GetPosR()));
	pProc->Print("角度   ({:.2f}/{:.2f}/{:.2f})\n\n", PRINT_VECTOR3(*GetRotate()));
}

//==================================================================================
// --- 設置処理 ---
//==================================================================================
void CPlayerCamera::SetCamera(void)
{ // カメラの設置
	CCamera::SetCamera();
}