//==================================================================================
// 
// プレイヤークラスのヘッダーファイル [player.h]
// Author : TENMA SAITO
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
class CField;

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

	static CPlayer *Create(std::string_view pFilename,
		const Vector3 &pos,
		const float fRadius);

	HRESULT Init(std::string_view pFilename,
		const Vector3 &pos,
		const float fRadius);
	void Uninit(void) override;
	void Update(void) override;
	void Draw(void) override;
	void Notified(CObject *pObject, std::string_view message) override;
	float GetRadius(void) const { return m_fRadius * m_fScaling; }

private:
	void CharactorMovable(void);
	void StatusChange(void);

	CField *m_pField = nullptr;			// 床へのポインタ
	Vector3 m_move = VECTOR3_NULL;		// 移動量
	Vector3 m_rotDest = VECTOR3_NULL;	// 目標角度
	Vector3 m_scl = VECTOR3_ONE;		// スケーリング倍率
	Matrix m_mtxRot;					// 回転マトリックス (保存用)
	Quaternion m_quat;					// クォータニオン
	Vector3 m_vecAxis = VECTOR3_NULL;	// 回転軸
	float m_fValueRot = 0.0f;			// 回転角
	float m_fRotSpeed = 0.0f;			// 回転速度
	float m_fRotResist = 0.0f;			// 減速係数
	float m_fRadius = 0.0f;				// 半径
	float m_fJump = 0.0f;				// 跳躍力
	float m_fCor = 0.0f;				// 反発係数
	float m_fScaling = 0.0f;			// 拡大倍率
	bool m_bJump = false;				// ジャンプフラグ
};
#endif