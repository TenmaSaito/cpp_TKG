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
	CItem(const int nPriority = ITEM_PRIORITY);
	~CItem();

	static CItem *Create(std::string_view path,
		const Vector3 &pos,
		const float fRadius,
		const int nLife);

	HRESULT Init(std::string_view path,
		const Vector3 &pos,
		const float fRadius,
		const int nLife);
	void Uninit(void) override;
	void Update(void) override;
	void Draw(void) override;

private:
	CPlayer *m_pPlayer = nullptr;		// プレイヤーへのポインタ
	CParticle *m_pParticle = nullptr;	// パーティクルへのポインタ
	CScore *m_pScore = nullptr;			// スコア表示へのポインタ
	Vector3 m_move = VECTOR3_NULL;	// 加速度
	float m_fRadius = 0.0f;			// モデルの獲得半径
	int m_nLife = 0;				// 体力
};
#endif