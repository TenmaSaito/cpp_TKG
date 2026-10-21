//==================================================================================
// 
// アイテム生成クラスのソースファイル [itemGenerator.cpp]
// Author : TENMA SAITO
// 
//==================================================================================
//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "itemGenerator.h"
#include "item.h"
#include "field.h"
#include "rand.h"

//**********************************************************************************
// *** 定数宣言 ***
//**********************************************************************************
namespace
{
	constexpr int MAX_ITEM_NUM = 10;		// 一度に出現するアイテムの総数
}

//==================================================================================
// --- コンストラクタ ---
//==================================================================================
CItemGenerator::CItemGenerator(const int nPriority) : CObject(nPriority)
{ // タイプ指定
	SetType(TYPE_ITEM_GENERATOR);
}

//==================================================================================
// --- デストラクタ ---
//==================================================================================
CItemGenerator::~CItemGenerator()
{
}

//==================================================================================
// --- 初期化処理 ---
//==================================================================================
HRESULT CItemGenerator::Init(void)
{
	return S_OK;
}

//==================================================================================
// --- 終了処理 ---
//==================================================================================
void CItemGenerator::Uninit(void)
{ // 親クラスの終了処理
	CObject::Release();
}

//==================================================================================
// --- 更新処理 ---
//==================================================================================
void CItemGenerator::Update(void)
{ 
	if (m_pField == nullptr)
	{ // 床を取得していなかったら取得
		m_pField = FindAnyObjectByType<CField>(FIELD_PRIORITY);
	}
	
	// アイテムが最大数分生成されているなら、
	// もしくは床へのポインタがnullならスキップ
	if (m_nNumItem > MAX_ITEM_NUM || m_pField == nullptr) return;

	CRand *pRand = CRand::GetInstance();		// 乱数生成機
	Vector3 pos;		// 生成位置
	float fRadius;		// 当たり判定の半径
	int nLife;			// 体力

	Vector4 boader;		// 境界線

	// 境界線を計算
	boader.x = m_pField->GetPosition()->x + (m_pField->GetSize()->x * 0.5f) - 20.0;		// 右端
	boader.y = m_pField->GetPosition()->y + (m_pField->GetSize()->x * 0.5f) - 20.0;		// 上端
	boader.z = m_pField->GetPosition()->y - (m_pField->GetSize()->x * 0.5f) + 20.0;		// 下端
	boader.w = m_pField->GetPosition()->x - (m_pField->GetSize()->x * 0.5f) + 20.0;		// 左端

	// 境界線内のどこかにランダムで生成
	pos.x = pRand->Generate(boader.w, boader.x);
	pos.y = 0.0f;
	pos.z = pRand->Generate(boader.z, boader.y);

	// 半径 + 体力を設定
	fRadius = 100.0f;
	nLife = pRand->Generate(100, 200);

	// アイテムを生成
	CItem *pItem = CItem::Create("data/MODEL/earth000.x",
		pos,
		fRadius,
		nLife);

	// 監視と親を設定
	pItem->AddObserver(this);
	pItem->SetParent(m_pField->GetMatrix());
	m_nNumItem++;		// アイテムの総数増加
}

//==================================================================================
// --- 描画処理 ---
//==================================================================================
void CItemGenerator::Draw(void)
{
}

//==================================================================================
// --- 通知受け取り処理 ---
//==================================================================================
void CItemGenerator::Notified(CObject *pObject, std::string_view message, const std::any &args)
{ // アイテム以外からの通知以外は無視する
	if (pObject->GetType() != TYPE_ITEM) return;
	// 死亡メッセージ以外は無視
	else if (message != NOTIFY_WHEN_DEATH) return;

	m_nNumItem--;		// アイテムの総数を減らす
}