//==================================================================================
// 
// マネージャクラスのヘッダーファイル [manager.h]
// Author : TENMA SAITO
// Date   : 2026/5/12
// 
//==================================================================================
#ifndef _MANAGER_H_		// インクルードガード
#define _MANAGER_H_

//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "main.h"

//**********************************************************************************
// *** 定数宣言 ***
//**********************************************************************************
constexpr int DEFAULT_APPLICATION_UPDATE_PER_SECOND = 60;		// 一秒間にUpdateする回数
constexpr int DEFAULT_APPLICATION_DRAW_PER_SECOND = 60;			// 一秒間にDrawする回数

//**********************************************************************************
// *** 前方宣言 ***
//**********************************************************************************
class CRenderer;
class CInputKeyboard;
class CInputMouse;	
class CJoypad;		
class CDebugProc;	
class CSound;		
class CLight;

//**********************************************************************************
// *** マネージャクラス ***
//**********************************************************************************
class CManager
{
public:
	HRESULT Init(const HINSTANCE hInstance,
		const HWND hWnd,
		const BOOL bWindow);
	void Uninit(void);
	void Update(void);
	void Draw(void);
	void SetFPS(const int nFPS) { m_nCountFPS = nFPS; }
	int GetFPS(void) { return m_nCountFPS; }
	void SetEnablePause(const bool bEnable);
	bool GetEnablePause(void);
	CRenderer *GetRenderer(void) const { return m_pRenderer.get(); }
	CInputKeyboard *GetInputKeyboard(void) const { return m_pInputKeyboard.get(); }
	CInputMouse *GetInputMouse(void) const { return m_pInputMouse.get(); }
	CJoypad *GetJoypad(void) const { return m_pJoypad.get(); }
	CDebugProc *GetDebugProc(void) const { return m_pDebugProc.get(); }
	CSound *GetSound(void) const { return m_pSound.get(); }
	CLight *GetLight(void) const { return m_pLight.get(); }
	HWND GetWindowHandle(void) const { return m_hWnd; }

	static CManager *GetInstance(void);
	static constexpr int SecToFrame(const int nSeconds) { return nSeconds * DEFAULT_APPLICATION_UPDATE_PER_SECOND; }
	static constexpr float SecToFrame(const float fSeconds) { return fSeconds * static_cast<float>(DEFAULT_APPLICATION_UPDATE_PER_SECOND); }
	static constexpr int FrameToSec(const int nFrame) { return nFrame / DEFAULT_APPLICATION_UPDATE_PER_SECOND; }
	static constexpr float FrameToSec(const float fFrame) { return fFrame / static_cast<float>(DEFAULT_APPLICATION_UPDATE_PER_SECOND); }
	static constexpr float RatioToSec(const float fT) { return FrameToSec(1.0f / fT); }
	static constexpr float SecToRatio(const float fSec) { return 1.0f / SecToFrame(fSec); }

private:
	CManager();
	~CManager();

	std::unique_ptr<CRenderer> m_pRenderer;				// レンダラーへのポインタ
	std::unique_ptr<CInputKeyboard> m_pInputKeyboard;	// キーボードオブジェクトへのポインタ
	std::unique_ptr<CInputMouse> m_pInputMouse;			// マウスオブジェクトへのポインタ
	std::unique_ptr<CJoypad> m_pJoypad;					// ジョイパッドオブジェクトへのポインタ
	std::unique_ptr<CDebugProc> m_pDebugProc;			// デバッグ表示オブジェクトへのポインタ
	std::unique_ptr<CSound> m_pSound;					// サウンドオブジェクトへのポインタ
	std::unique_ptr<CLight> m_pLight;					// ライトオブジェクトへのポインタ
	HWND m_hWnd;			// ウィンドウハンドル
	int m_nCountFPS;		// FPSカウント
	int m_nCounterFrame;	// フレームカウンター
};
#endif