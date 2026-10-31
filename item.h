//==================================================================================
// 
// アイテムクラスのヘッダーファイル [item.h]
// Author : TENMA SAITO
// 
//==================================================================================
#ifndef _ITEM_H_		// インクルードガード
#define _ITEM_H_

//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "objectX.h"

//**********************************************************************************
// *** 前方宣言 ***
//**********************************************************************************
class CPlayer;
class CParticle;
class CNumber3D;
class CObjectBillboard3D;
class CScore;

//**********************************************************************************
// *** 定数宣言 ***
//**********************************************************************************
constexpr int ITEM_PRIORITY = DEFAULT_OBJ_PRIORITY;		// アイテムの優先順位

//**********************************************************************************
// *** アイテムクラス ***
//**********************************************************************************
class CItem : public CObjectX
{
public:
	// アイテム(食べ物)の種類
	enum class TYPE
	{
		NONE = 0,	// 指定なし
		RICE,		// ご飯
		SIDE_DISH,	// おかず
		SOUP,		// 汁物
		MAX
	};

	CItem(const int nPriority = ITEM_PRIORITY);
	~CItem();

	static CItem *Create(std::string_view path,
		const Vector3 &pos,
		const float fRadius,
		const int nLife,
		const TYPE type);

	HRESULT Init(std::string_view path,
		const Vector3 &pos,
		const float fRadius,
		const int nLife,
		const TYPE type);
	void Uninit(void) override;
	void Update(void) override;
	void Draw(void) override;
	bool IsEqualType(const TYPE type) const { return m_type == type; }
	static constexpr std::string_view GetPath(const TYPE type) { return c_asModelPath[static_cast<UINT>(type)]; }

private:
	CObjectBillboard3D *m_pIcon = nullptr;		// 表示アイコン
	CPlayer *m_pPlayer = nullptr;		// プレイヤーへのポインタ
	CParticle *m_pParticle = nullptr;	// パーティクルへのポインタ
	CScore *m_pScore = nullptr;			// スコア表示へのポインタ
	Vector3 m_move = VECTOR3_NULL;	// 加速度
	float m_fRadius = 0.0f;			// モデルの獲得半径
	int m_nLife = 0;				// 体力
	TYPE m_type = TYPE::NONE;		// 自身のタイプ
	inline static constexpr std::string_view c_asModelPath[static_cast<UINT>(TYPE::MAX)] =
	{
		"",
		"data/MODEL/rice000.x",
		"data/MODEL/humbarg.x",
		"data/MODEL/miso_soup.x",
	};
};
#endif