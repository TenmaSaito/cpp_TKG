//==================================================================================
// 
// オブジェクトクラスのヘッダーファイル [object.h]
// Author : TENMA SAITO
// Date   : 2026/5/8
// 
//==================================================================================
#ifndef _OBJECT_H_		// インクルードガード
#define _OBJECT_H_

//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "main.h"
#include <deque>

//**********************************************************************************
// *** マクロ定義 ***
//**********************************************************************************
#define DEBUG_ASSERT_TYPE_NONE				// オブジェクトにタイプが指定されていなかった場合、アサーション

//**********************************************************************************
// *** 定数宣言 ***
//**********************************************************************************
constexpr int MAX_OBJPRIORITY = 8;			// 優先順位の総数
constexpr int DEFAULT_BG_PRIORITY = 0;		// 背景の優先順位
constexpr int DEFAULT_OBJ_PRIORITY = 3;		// オブジェクトの優先順位
constexpr int DEFAULT_EFFECT_PRIORITY = 4;	// エフェクト関連の優先順位
constexpr int DEFAULT_ADD_PRIORITY = 5;		// 加算合成関連の優先順位
constexpr int DEFAULT_UI_PRIORITY = 6;		// UI関連の優先順位
constexpr int FINALLY_PRIORITY = 7;			// 全てのオブジェクトが描画された後の最終優先順位
constexpr std::string_view NOTIFY_WHEN_DEATH = "Death";			// 死んだ際に通知される文字列

//**********************************************************************************
// *** オブジェクトクラス ***
//**********************************************************************************
class CObject
{
public:
	// オブジェクトの種類
	typedef enum
	{
		TYPE_NONE = 0,		// 指定無し (アサーション対象)
		TYPE_OBJ_3D,		// Object3D
		TYPE_OBJ_X,			// ObjectX
		TYPE_OBJ_LINE,		// ObjectLine
		TYPE_FIELD,			// 床
		TYPE_WALL,			// 壁
		TYPE_PLAYER,		// プレイヤー
		TYPE_SHADOW,		// 影
		TYPE_ITEM,			// アイテム
		TYPE_MAX
	} TYPE;

	CObject(const int nPriority = DEFAULT_OBJ_PRIORITY);
	virtual ~CObject();

	virtual HRESULT Init(void) { return S_OK; }
	virtual void Uninit(void) {}
	virtual void Update(void) {}
	virtual void Draw(void) {}
	virtual void Notified(CObject *pObject, std::string_view message) {}

	void NotifyAll(std::string_view message);
	void SetType(const TYPE type) { m_type = type; }
	TYPE GetType(void) const { return m_type; }
	bool IsDeath(void) const { return m_bDeath; }
	void AddObserver(CObject *pObserve) { m_vpObserver.push_back(pObserve); }
	void RemoveObserver(CObject *pObserve) { m_vpObserver.erase(std::remove(m_vpObserver.begin(), m_vpObserver.end(), pObserve), m_vpObserver.end()); }
	static CObject* GetTop(const int nPriority) { return m_apTop[nPriority]; }
	CObject *GetNext(void) const { return m_pNext; }
	static int GetNumAll(void) { return m_nNumAll; }
	static void ReleaseAll(void);
	static void UpdateAll(void);
	static void DrawAll(void);
	static void FlagCheckAll(void);
	static CObject *FindAnyObjectByType(const TYPE type);
	static CObject *FindAnyObjectByType(const int nPriority, const TYPE type);
	static std::deque<CObject*> FindObjectsByType(const TYPE type);
	static std::deque<CObject*> FindObjectsByType(const int nPriority, const TYPE type);

protected:
	void Release(void);

private:

	void AddList(void);
	void RemoveList(void);
	void NofifyAllWhenDeath(void);

	static CObject *m_apTop[MAX_OBJPRIORITY];		// 先頭オブジェクトへのポインタ
	static CObject *m_apCur[MAX_OBJPRIORITY];		// 最後尾オブジェクトへのポインタ
	CObject *m_pPrev;			// 前のオブジェクトへのポインタ
	CObject *m_pNext;			// 次のオブジェクトへのポインタ
	static int m_nNumAll;		// オブジェクトの総数
	int m_nPriority;	// 優先順位の位置
	TYPE m_type;		// オブジェクトタイプ
	bool m_bDeath;		// 死亡フラグ
	std::vector<CObject*> m_vpObserver;	// 自身を管理しているオブジェクトへのポインタ
};
#endif