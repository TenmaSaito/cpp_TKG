//==================================================================================
// 
// アイテムクラスのソースファイル [item.cpp]
// Author : TENMA SAITO
// 
//==================================================================================
//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "item.h"
#include "model.h"
#include "player.h"
#include "vec3math.h"
#include "matrix.h"
#include "manager.h"
#include "light.h"
#include "particle.h"
#include "texture.h"
#include "score.h"

// 定数宣言
namespace
{
	constexpr int ADD_SCORE = 10;		// スコアの加算値
}

//==================================================================================
// --- アイテムの生成処理 ---
//==================================================================================
CItem *CItem::Create(std::string_view path,
	const Vector3 &pos,
	const float fRadius,
	const int nLife)
{
	CItem *pItem = new CItem;		// 生成したオブジェクトへのポインタ
	if (pItem != nullptr)
	{ // 生成出来ていれば初期化
		pItem->Init(path, pos, fRadius, nLife);
	}

	return pItem;
}

//==================================================================================
// --- コンストラクタ ---
//==================================================================================
CItem::CItem(const int nPriority) : CObjectX(nPriority)
{ // タイプ指定
	SetType(TYPE_ITEM);
}

//==================================================================================
// --- デストラクタ ---
//==================================================================================
CItem::~CItem()
{
}

//==================================================================================
// --- 初期化処理 ---
//==================================================================================
HRESULT CItem::Init(std::string_view path,
	const Vector3 &pos,
	const float fRadius,
	const int nLife)
{ // モデル生成
	CObjectX::Init(path, pos, VECTOR3_NULL);

	m_fRadius = fRadius;		// 半径保存
	m_nLife = nLife;			// 体力保存
	return S_OK;
}

//==================================================================================
// --- 終了処理 ---
//==================================================================================
void CItem::Uninit(void)
{ 
	if (m_pParticle != nullptr)
	{ // パーティクルを破棄
		m_pParticle->Uninit();
		m_pParticle = nullptr;
	}

	// 親クラスの終了
	CObjectX::Release();
}

//==================================================================================
// --- 更新処理 ---
//==================================================================================
void CItem::Update(void)
{
	if (m_pPlayer == nullptr)
	{ // プレイヤーへのポインタが未取得の場合、取得
		m_pPlayer = CObject::FindAnyObjectByType<CPlayer>(PLAYER_PRIORITY);
	}

	if (m_pScore == nullptr)
	{ // スコア表示へのポインタが未取得の場合、取得
		m_pScore = CObject::FindAnyObjectByType<CScore>(SCORE_PRIORITY);
	}

	if (m_pPlayer == nullptr || m_pScore == nullptr)
	{ // プレイヤーがnullの場合スキップ
		return;
	}

	float fLength = Vec3::Length(*m_pPlayer->GetPosition(), *GetPosition());
	if (fLength <= m_fRadius && m_pPlayer->IsAutoRotating())
	{ // 獲得半径内にいるかつプレイヤーが回転していれば、獲得ゲージを減らすため体力減少
		m_nLife--;
		m_pScore->AddScore(ADD_SCORE);
		
		if (m_pParticle == nullptr)
		{ // パーティクルを生成していないなら、新規生成
			CParticle::Setting setting = {};
			setting.pos = *GetPosition();
			setting.move = Vector3(0.1f, 0.1f, 0.1f);
			setting.scale = Vector2(50.0f, 50.0f);
			setting.color = Color(1.0f, 1.0f, 1.0f, 0.5f);
			setting.nLife = INT_MAX;
			setting.nNumEffectFrame = 3;
			setting.moveVariation = Vector3(5.0f, 5.0f, 5.0f);
			setting.bEnableAlphaBlending = true;

			m_pParticle = CParticle::Create(setting);
			m_pParticle->BindTexture(CTexture::GetInstance()->Register("data/TEXTURE/effect000.jpg"));
		}

		// パーティクルを有効化
		m_pParticle->SetEnable(true);
	}
	else if (m_pParticle != nullptr)
	{ // パーティクルを無効化
		m_pParticle->SetEnable(false);
	}

	if (m_nLife <= 0)
	{ // 体力が尽きたら死亡
		Uninit();
	}
}

//==================================================================================
// --- 描画処理 ---
//==================================================================================
void CItem::Draw(void)
{ 
	CManager *pManager = CManager::GetInstance();			// マネージャーへのポインタ
	Matrix mtxShadow;		// シャドウマトリックス
	
	// 描画
	CObjectX::Draw();

	Vector3 nor = Vector3(0.0f, 1.0f, 0.0f);		// 床の法線

	// 法線を親マトリックスで計算
	D3DXVec3TransformNormal(&nor, &nor, GetParent());

	// 床のシャドウマトリックスを生成
	Mtx::CreateShadow(GetMatrix(),
		Vector3(0.0f, 0.1f, 0.0f),
		nor,
		*pManager->GetLight()->GetLight(0),
		&mtxShadow);

	// 影を描画
	CObjectX::DrawShadow(mtxShadow);
}