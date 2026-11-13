//==================================================================================
// 
// タイトルシーンのソースファイル [title.cpp]
// Author : TENMA SAITO
// 
//==================================================================================
//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "title.h"
#include "object2D.h"
#include "objectX.h"
#include "manager.h"
#include "input.h"
#include "joypad.h"
#include "texture.h"
#include "camera.h"
#include "foodBackground.h"
#include "sound.h"
#include "util.h"

//**********************************************************************************
// *** 定数宣言 ***
//**********************************************************************************
namespace
{
	constexpr std::string_view LOGO_PATH = "data/TEXTURE/title_logo.png";		// タイトルロゴ
	const Vector3 ROTATE_SPD = Vector3(0.0f, 0.01f, 0.005f);		// キャラクターの回転速度
}

//==================================================================================
// --- コンストラクタ ---
//==================================================================================
CTitle::CTitle()
{
}

//==================================================================================
// --- デストラクタ ---
//==================================================================================
CTitle::~CTitle()
{
}

//==================================================================================
// --- 初期化処理 ---
//==================================================================================
HRESULT CTitle::Init(void)
{ // 開始処理
	Start();

	return S_OK;
}

//==================================================================================
// --- 終了処理 ---
//==================================================================================
void CTitle::Uninit(void)
{
}

//==================================================================================
// --- 更新処理 ---
//==================================================================================
void CTitle::Update(void)
{
	CManager *pManager = CManager::GetInstance();
	CInputKeyboard *pKeyboard = pManager->GetInputKeyboard();
	CJoypad *pJoypad = pManager->GetJoypad();

	if ((pKeyboard->GetTrigger(DIK_RETURN)
		|| pJoypad->GetTrigger(CJoypad::KEY_A)
		|| pJoypad->GetTrigger(CJoypad::KEY_START))
		&& pManager->IsTransition() == false)
	{ // 遷移中でなく、Enterを押されたら遷移
		CManager::GetInstance()->GetSound()->Play(CSound::LABEL_SE_BITE);
		pManager->SetTransition(MODE_GAME);
	}

	if (m_pCharacter == nullptr) return;

	const Vector3 *pRot = m_pCharacter->GetRotation();		// 現在の角度
	m_pCharacter->SetRotation(Vector3(Util::FixedRotation(pRot->x + ROTATE_SPD.x),
		Util::FixedRotation(pRot->y + ROTATE_SPD.y),
		Util::FixedRotation(pRot->z + ROTATE_SPD.z)));
}

//==================================================================================
// --- 描画処理 ---
//==================================================================================
void CTitle::Draw(void)
{
}

//==================================================================================
// --- 開始時処理 ---
//==================================================================================
void CTitle::Start(void)
{
	CObject2D::Info info = {};

	info.pos = SCREEN_MIDDLE;
	info.size = SCREEN_SIZE;

	CObject2D::Create(info)->SetColor(Color(1.0f, 1.0f, 1.0f, 0.5f));

	info.pos = Vector3(SCREEN_MIDDLE.x, 600.0f, 0.0f);
	info.size = Vector2(300.0f, 50.0f);

	CObject2D::Create(info)->BindTexture(CTexture::GetInstance()->Register("data/TEXTURE/pressEnter.png"));
	CCamera::Create(Vector3(0.0f, 300.0f, -300.0f), Vector3(0.0f, 300.0f, 0.0f));

	info.pos = Vector3(SCREEN_MIDDLE.x, 275.0f, 0.0f);
	info.size = Vector2(650.0f, 550.0f);
	CObject2D::Create(info)->BindTexture(CTexture::GetInstance()->Register(LOGO_PATH));

	// バックの食べ物出現
	CFoodBackground::Create();

	// 回転させるキャラクターを生成
	m_pCharacter = CObjectX::Create("data/MODEL/quaterni_kun.x", Vector3(50.0f, 300.0f, -200.0f));

	// BGM再生
	CManager::GetInstance()->GetSound()->Play(CSound::LABEL_BGM_TITLE);
}