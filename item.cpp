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
#include "renderer.h"
#include "light.h"
#include "particle.h"
#include "texture.h"
<<<<<<< Updated upstream
=======
#include "score.h"
#include "objectBillboard3D.h"

// 定数宣言
namespace
{
	constexpr int ADD_SCORE = 10;		// スコアの加算値
}
>>>>>>> Stashed changes

//==================================================================================
// --- アイテムの生成処理 ---
//==================================================================================
CItem *CItem::Create(std::string_view path,
	const Vector3 &pos,
	const float fRadius,
	const int nLife,
	const TYPE type)
{
	CItem *pItem = new CItem;		// 生成したオブジェクトへのポインタ
	if (pItem != nullptr)
	{ // 生成出来ていれば初期化
		pItem->Init(path, pos, fRadius, nLife, type);
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
	const int nLife,
	const TYPE type)
{ // モデル生成
	CObjectX::Init(path, pos, VECTOR3_NULL);

	m_fRadius = fRadius;		// 半径保存
	m_nLife = nLife;			// 体力保存
	m_type = type;		// タイプ保存

	// 引数のタイプのアイコンを生成
	m_pIcon = CObjectBillboard3D::Create(Vector3(0.0f, 50.0f, 0.0f),
		VECTOR3_NULL,
		VECTOR2_ONE * 50.0f);

	// 親を登録
	m_pIcon->SetParent(GetMatrix());

	// 描画前設定を追加
	m_pIcon->AddRenderState({ D3DRS_ZFUNC, D3DCMP_ALWAYS }, false);
	m_pIcon->AddRenderState({ D3DRS_ZWRITEENABLE, FALSE }, false);
	m_pIcon->AddRenderState({ D3DRS_ZENABLE, FALSE }, false);

	// 描画後設定を追加
	m_pIcon->AddRenderState({ D3DRS_ZFUNC, D3DCMP_LESSEQUAL }, false);
	m_pIcon->AddRenderState({ D3DRS_ZWRITEENABLE, TRUE }, false);
	m_pIcon->AddRenderState({ D3DRS_ZENABLE, TRUE }, false);
	return S_OK;
}

//==================================================================================
// --- 終了処理 ---
//==================================================================================
void CItem::Uninit(void)
{ 
	if (m_pIcon)
	{ // アイコンを破棄
		m_pIcon->Uninit();
		m_pIcon = nullptr;
	}

	// 親クラスの終了
	CObjectX::Uninit();
}

//==================================================================================
// --- 更新処理 ---
//==================================================================================
void CItem::Update(void)
{
	if (m_pPlayer == nullptr)
	{ // プレイヤーへのポインタが取得出来ていない場合、プレイヤーを検索
		m_pPlayer = CObject::FindAnyObjectByType<CPlayer>(PLAYER_PRIORITY);
	}

	if (m_pPlayer == nullptr)
	{ // プレイヤーがnullの場合スキップ
		return;
	}

	float fLength = Vec3::Length(*m_pPlayer->GetPosition(), *GetPosition());
	if (fLength <= m_fRadius + m_pPlayer->GetRadius())
	{ // 獲得半径内にいるかつプレイヤーが回転していれば、獲得ゲージを減らすため体力減少
		m_nLife--;
		
		if (m_pParticle == nullptr)
		{ // パーティクルを生成していないなら、新規生成
			CParticle::Setting setting = {};
			setting.pos = *GetPosition();
			setting.move = VECTOR3_ONE * 0.1f;
			setting.scale = Vector2(50.0f, 50.0f);
			setting.color = Color(1.0f, 1.0f, 1.0f, 0.5f);
			setting.nLife = 30;
			setting.nNumEffectFrame = 10;
			setting.moveVariation = VECTOR3_ONE * 10.0f;
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
	{ // 体力が尽きたら取得時処理を呼び出して死亡
		m_pPlayer->Eat();
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