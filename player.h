//==================================================================================
// 
// プレイヤークラスのヘッダーファイル [player.h]
// Author : TENMA SAITO
// Date   : 2026/9/28
// 
//==================================================================================
#ifndef _PLAYER_H_
#define _PLAYER_H_

//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "objectX.h"

//**********************************************************************************
// *** 前方宣言 ***
//**********************************************************************************
class CShadow;

//**********************************************************************************
// *** 定数宣言 ***
//**********************************************************************************
constexpr int PLAYER_PRIORITY = DEFAULT_OBJX_PRIORITY;		// プレイヤーの優先順位

//**********************************************************************************
// *** プレイヤークラス ***
//**********************************************************************************
class CPlayer : public CObjectX
{
public:
	CPlayer(int nPriority = PLAYER_PRIORITY);
	~CPlayer();

	static CPlayer *Create(const char *pFilename,
		const Vector3 &pos,
		const Vector3 &rot);

	HRESULT Init(const char *pFilename,
		const Vector3 &pos,
		const Vector3 &rot);
	void Uninit(void) override;
	void Update(void) override;
	void Draw(void) override;
	void Notified(CObject *pObject, std::string_view message) override;

private:
	void CharactorMovable(void);

	CShadow *m_pShadow = nullptr;		// 影へのポインタ
	Vector3 m_move = VECTOR3_NULL;		// 移動量
	Vector3 m_rotDest = VECTOR3_NULL;	// 目標角度
	bool m_bJump = false;				// ジャンプフラグ
};
#endif