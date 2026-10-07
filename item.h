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
#include "object.h"

//**********************************************************************************
// *** 前方宣言 ***
//**********************************************************************************
class CModel;
class CPlayer;

//**********************************************************************************
// *** 定数宣言 ***
//**********************************************************************************
constexpr int ITEM_PRIORITY = DEFAULT_OBJ_PRIORITY;		// アイテムの優先順位

//**********************************************************************************
// *** アイテムクラス ***
//**********************************************************************************
class CItem : public CObject
{
public:
	CItem(const int nPriority = ITEM_PRIORITY);
	~CItem();

	static CItem *Create(std::string_view path,
		const Vector3 &pos,
		const float fRadius);

	HRESULT Init(std::string_view path,
		const Vector3 &pos,
		const float fRadius);
	void Uninit(void) override;
	void Update(void) override;
	void Draw(void) override;

private:
	std::unique_ptr<CModel> m_pModel;		// モデルへのポインタ
	CPlayer *m_pPlayer = nullptr;	// プレイヤーへのポインタ
	Vector3 m_move = VECTOR3_NULL;	// 加速度
	float m_fRadius = 0.0f;			// モデルの半径
	bool m_bLand = false;			// 着地フラグ
};
#endif