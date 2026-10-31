//==================================================================================
// 
// 画面下に表示されるUIクラスのヘッダーファイル [nextInfo.h]
// Author : TENMA SAITO
// 
//==================================================================================
#ifndef _NEXTINFO_H_
#define _NEXTINFO_H_

//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "object.h"

//**********************************************************************************
// *** 前方宣言 ***
//**********************************************************************************
class CObject2D;

//**********************************************************************************
// *** 画面下に表示されるUIクラス ***
//**********************************************************************************
class CNextInfo : public CObject
{
public:
	CNextInfo();
	~CNextInfo();

	static CNextInfo *Create(void);

	HRESULT Init(void);
	void Uninit(void) override;
	void Update(void) override;
	void SetNext(CObject2D *pNext);
	void Angry(const int nAngryFrame = ANGRY_FRAME);

private:
	static constexpr int ANGRY_FRAME = 120;		// 怒るフレーム数
	CObject2D *m_pBase = nullptr;		// 一番下に表示されるコメント
	CObject2D *m_pNext = nullptr;		// 次に取得すべきアイテム
	bool m_bIsAngry = true;		// 次に取得すべきアイテムを表示するか
	int m_nAngryCount = 0;		// 怒ったフレーム数
};
#endif