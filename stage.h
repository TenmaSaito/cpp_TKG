//==================================================================================
// 
// ステージクラスのヘッダーファイル [stage.h]
// Author : TENMA SAITO
// 
//==================================================================================
#ifndef _STAGE_H_
#define _STAGE_H_

//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "field.h"

//**********************************************************************************
// *** 前方宣言 ***
//**********************************************************************************
class CPlayer;

//**********************************************************************************
// *** 定数宣言 ***
//**********************************************************************************
constexpr int STAGE_PRIORITY = FIELD_PRIORITY;		// ステージの優先順位

//**********************************************************************************
// *** ステージクラス ***
//**********************************************************************************
class CStage : public CField
{
public:
	CStage(const int nPriority = STAGE_PRIORITY);
	~CStage();

	static CStage *Create(const Vector3 &pos,
		const Vector3 &rot,
		const Vector2 &size);

	HRESULT Init(const Vector3 &pos,
		const Vector3 &rot,
		const Vector2 &size);
	void Uninit(void) override;
	void Update(void) override;
	void Draw(void) override;
	void Notified(CObject *pObject, std::string_view message, const std::any &args) override;

private:
	bool LeanInput(Vector3 *pRot);

	CPlayer *m_pPlayer = nullptr;		// プレイヤーへのポインタ
};
#endif