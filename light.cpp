//==================================================================================
// 
// ライトクラスのソースファイル [camera.cpp]
// Author : TENMA SAITO
// Date   : 2026/6/1
// 
//==================================================================================
//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "light.h"
#include "manager.h"
#include "renderer.h"
#include "input.h"
#include "joypad.h"
#include "debugproc.h"
#include "vec3math.h"

//**********************************************************************************
// *** マクロ定義 ***
//**********************************************************************************
#define DEFAULT_FOVY		(45.0f)		// 視野角
#define DEFAULT_ZN			(0.0f)		// 最短距離
#define DEFAULT_ZF			(10000.0f)	// 最遠距離

//==================================================================================
// --- コンストラクタ ---
//==================================================================================
CLight::CLight()
{
}

//==================================================================================
// --- デストラクタ ---
//==================================================================================
CLight::~CLight()
{
}

//==================================================================================
// --- 初期化処理 ---
//==================================================================================
void CLight::Init(void)
{
	LPDIRECT3DDEVICE9 pDevice = CManager::GetInstance()->GetRenderer()->GetDevice();		// デバイスへのポインタ
	Vector3 vecDir[3];			// ライトの方向ベクトル

	// ライトの情報を初期化
	memset(&m_aLight[0], 0, sizeof(m_aLight));

	// ライトの種類を設定
	m_aLight[0].Type = D3DLIGHT_DIRECTIONAL;
	m_aLight[1].Type = D3DLIGHT_DIRECTIONAL;
	m_aLight[2].Type = D3DLIGHT_DIRECTIONAL;

	// ライトの拡散光を設定
	m_aLight[0].Diffuse = Color(1.0f, 1.0f, 1.0f, 1.0f);
	m_aLight[1].Diffuse = Color(0.7f, 0.7f, 0.7f, 1.0f);
	m_aLight[2].Diffuse = Color(0.3f, 0.3f, 0.3f, 1.0f);

	// ライトの方向を設定
	vecDir[0] = Vector3(0.0f, -0.8f, -0.4f);
	D3DXVec3Normalize(&vecDir[0], &vecDir[0]);
	m_aLight[0].Direction = vecDir[0];

	vecDir[1] = Vector3(-0.3f, 0.4f, 0.5f);
	D3DXVec3Normalize(&vecDir[1], &vecDir[1]);
	m_aLight[1].Direction = vecDir[1];

	vecDir[2] = Vector3(0.2f, 0.1f, 0.1f);
	D3DXVec3Normalize(&vecDir[2], &vecDir[2]);
	m_aLight[2].Direction = vecDir[2];

	// ライトを設定する
	pDevice->SetLight(0, &m_aLight[0]);
	pDevice->SetLight(1, &m_aLight[1]);
	pDevice->SetLight(2, &m_aLight[2]);

	// ライトを有効にする
	pDevice->LightEnable(0, TRUE);
	pDevice->LightEnable(1, TRUE);
	pDevice->LightEnable(2, TRUE);

	// 操作するライトのインデックスを指定
	m_nIdxLight = 0;
}

//==================================================================================
// --- 終了処理 ---
//==================================================================================
void CLight::Uninit(void)
{
}

//==================================================================================
// --- 更新処理 ---
//==================================================================================
void CLight::Update(void)
{
	CManager *pManager = CManager::GetInstance();			// マネージャーへのポインタ
	LPDIRECT3DDEVICE9 pDevice = pManager->GetRenderer()->GetDevice();		// デバイスへのポインタ
	auto pKeyboard = pManager->GetInputKeyboard();			// キーボードへのポインタ
	auto pProc = CManager::GetInstance()->GetDebugProc();		// デバッグ表示へのポインタ
	Vector3 vecDir = m_aLight[m_nIdxLight].Direction;	// ライトの向き 

	// 縦回転
	if (pKeyboard->GetPress(DIK_J))
	{
		m_rot.y -= 0.05f;
	}
	else if (pKeyboard->GetPress(DIK_L))
	{
		m_rot.y += 0.05f;
	}

	// 横回転
	if (pKeyboard->GetPress(DIK_I))
	{
		m_rot.z += 0.05f;
	}
	else if (pKeyboard->GetPress(DIK_K))
	{
		m_rot.z -= 0.05f;
	}

	// ベクトルを求める
	vecDir = Vec3::Arc(1.0f, m_rot.y, m_rot.z);

	// 正規化
	D3DXVec3Normalize(&vecDir, &vecDir);
	m_aLight[m_nIdxLight].Direction = vecDir;

	// ライトを再設定
	pDevice->SetLight(m_nIdxLight, &m_aLight[m_nIdxLight]);

	pProc->Print("====== ライト ======\n");
	pProc->Print("垂直回転 ({:.2f}) [I/K]\n", m_rot.z);
	pProc->Print("水平回転 ({:.2f}) [J/L]\n\n", m_rot.y);
}