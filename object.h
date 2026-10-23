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
#include <any>

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
		TYPE_OBJ_2D,		// Object2D
		TYPE_OBJ_3D,		// Object3D
		TYPE_OBJ_BILLBOARD_3D,	// Object3D[Billboard]
		TYPE_OBJ_X,			// ObjectX
		TYPE_OBJ_X_AFTERIMAGE,	// ObjectXの残像
		TYPE_OBJ_LINE,		// ObjectLine
		TYPE_FIELD,			// 床
		TYPE_WALL,			// 壁
		TYPE_PLAYER,		// プレイヤー
		TYPE_SHADOW,		// 影
		TYPE_ITEM,			// アイテム
		TYPE_STAGE,			// ステージ
		TYPE_TIMER,			// タイマー
		TYPE_PARTICLE,		// パーティクル
		TYPE_ITEM_GENERATOR,	// アイテム生成機
		TYPE_SCORE,			// スコア
		TYPE_MAX
	} TYPE;

	CObject(const int nPriority = DEFAULT_OBJ_PRIORITY);
	virtual ~CObject();

	virtual HRESULT Init(void) { return S_OK; }
	virtual void Uninit(void) {}
	virtual void Update(void) {}
	virtual void Draw(void) {}
	virtual void Notified(CObject *pObject, std::string_view message, const std::any &args = {}) {}

	void NotifyAll(std::string_view message, const std::any &args = {});
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
	template<class T> static T *FindAnyObjectByType(void);
	template<class T> static T *FindAnyObjectByType(const int nPriority);
	static std::vector<CObject*> FindObjectsByType(const TYPE type);
	static std::vector<CObject*> FindObjectsByType(const int nPriority, const TYPE type);
	template<class T> static std::vector<T*> FindObjectsByType(void);
	template<class T> static std::vector<T*> FindObjectsByType(const int nPriority);

protected:
	void Release(void);

private:
	void AddList(void);
	void RemoveList(void);
	void NotifyAllWhenDeath(void);

	inline static CObject *m_apTop[MAX_OBJPRIORITY] = { nullptr };		// 先頭オブジェクトへのポインタ
	inline static CObject *m_apCur[MAX_OBJPRIORITY] = { nullptr };		// 最後尾オブジェクトへのポインタ
	inline static int m_nNumAll = 0;	// オブジェクトの総数
	std::vector<CObject*> m_vpObserver = {};	// 自身を管理しているオブジェクトへのポインタ
	CObject *m_pPrev = nullptr;			// 前のオブジェクトへのポインタ
	CObject *m_pNext= nullptr;			// 次のオブジェクトへのポインタ
	int m_nPriority = 0;			// 優先順位の位置
	TYPE m_type = TYPE_NONE;		// オブジェクトタイプ
	bool m_bDeath = false;			// 死亡フラグ
};

//==================================================================================
// --- オブジェクトの検索処理 (タイプ指定 : 初めに見つかったポインタ) ---
//==================================================================================
template<class T> T *CObject::FindAnyObjectByType(void)
{
	T *pCast = nullptr;		// キャスト後のポインタ

	for (int nCntPriority = 0; nCntPriority < MAX_OBJPRIORITY; nCntPriority++)
	{ // 全プライオリティを検索
		pCast = FindAnyObjectByType<T>(nCntPriority);

		// ポインタがnullでなければ終了
		if (pCast != nullptr) break;
	}

	return pCast;
}

//==================================================================================
// --- オブジェクトの検索処理 (タイプ・優先順位指定 : 初めに見つかったポインタ) ---
//==================================================================================
template<class T> T *CObject::FindAnyObjectByType(const int nPriority)
{ // 範囲外ならアサーション
	assert(nPriority >= 0 && nPriority < MAX_OBJPRIORITY);

	CObject *pObject = m_apTop[nPriority];		// オブジェクトへのポインタ
	T *pCast = nullptr;		// キャスト後のポインタ

	while (pObject != nullptr)
	{ // オブジェクトへのポインタがnullではない間ループ
		CObject *pObjectNext = pObject->GetNext();		// 次のオブジェクトへのポインタ

		// タイプが同じだった場合、ループ終了
		pCast = dynamic_cast<T*>(pObject);
		if (pCast != nullptr) break;

		pObject = pObjectNext;		// ポインタを進める
	}

	// 検索結果を返す
	return pCast;
}

//==================================================================================
// --- オブジェクトの検索処理 (タイプ指定 : 見つかったポインタ全て) ---
//==================================================================================
template<class T> std::vector<T*> CObject::FindObjectsByType(void)
{
	std::vector<T*> vpObject;				// 見つかったオブジェクトへのポインタ

	for (int nCntPriority = 0; nCntPriority < MAX_OBJPRIORITY; nCntPriority++)
	{ // 全プライオリティを検索
		std::vector<T*> vpFind;		// 今回のプライオリティで見つかった配列

		// 検索
		vpFind = FindObjectsByType<T>(nCntPriority);

		// 配列の要素を挿入
		vpObject.insert(vpObject.end(), vpFind.begin(), vpFind.end());
	}

	// 検索結果を返す
	return vpObject;
}

//==================================================================================
// --- オブジェクトの検索処理 (優先順位, タイプ指定 : 見つかったポインタ全て) ---
//==================================================================================
template<class T> std::vector<T*> CObject::FindObjectsByType(const int nPriority)
{ // 範囲外ならアサーション
	assert(nPriority >= 0 && nPriority < MAX_OBJPRIORITY);

	CObject *pObject = m_apTop[nPriority];		// オブジェクトへのポインタ
	std::vector<T*> vpObject;					// 見つかったオブジェクトへのポインタ

	while (pObject != nullptr)
	{ // オブジェクトへのポインタがnullではない間ループ
		CObject *pObjectNext = pObject->GetNext();		// 次のオブジェクトへのポインタ

		// タイプが同じだった場合、ポインタを追加
		T *pCast = dynamic_cast<T*>(pObject);
		if (pCast != nullptr) vpObject.push_back(pCast);

		pObject = pObjectNext;		// ポインタを進める
	}

	// 検索結果を返す
	return vpObject;
}
#endif