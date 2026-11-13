//==================================================================================
//
// ランキングシーンのソースファイル [ranking.cpp]
// Author : TENMA SAITO
//
//==================================================================================
//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "ranking.h"
#include "object2D.h"
#include "manager.h"
#include "sound.h"
#include "input.h"
#include "joypad.h"
#include "texture.h"
#include "camera.h"
#include "rankingClient.h"
#include "score.h"
#include "foodBackground.h"

//**********************************************************************************
// *** 定数宣言 ***
//**********************************************************************************
namespace
{
	constexpr std::string_view SERVER_IPADDRESS = "127.0.0.1";		// ランキングサーバーのIPアドレス
	constexpr u_short SERVER_PORT = 22333;							// ランキングサーバーのポート番号
	constexpr const char *NUMBER_TEXTURE = "data/TEXTURE/voltNumber003.png";	// 数値のテクスチャ
	constexpr int RANK_DIGIT = 1;			// 順位の桁数
	constexpr int SCORE_DIGIT = 8;			// スコアの桁数
	const Vector2 RANK_SIZE = Vector2(60.0f, 80.0f);		// 順位の表示サイズ
	const Vector2 SCORE_SIZE = Vector2(480.0f, 80.0f);		// スコアの表示サイズ
	constexpr float RANK_SPACE = 40.0f;		// 順位とスコアの間隔
	constexpr float LINE_SPACE = 110.0f;	// 各順位の行間
	constexpr float TOP_POS_Y = 100.0f;		// 1位のY座標
}

//==================================================================================
// --- コンストラクタ ---
//==================================================================================
CRanking::CRanking()
{
}

//==================================================================================
// --- デストラクタ ---
//==================================================================================
CRanking::~CRanking()
{
}

//==================================================================================
// --- 初期化処理 ---
//==================================================================================
HRESULT CRanking::Init(void)
{ // 開始処理
	Start();
	return S_OK;
}

//==================================================================================
// --- 終了処理 ---
//==================================================================================
void CRanking::Uninit(void)
{
}

//==================================================================================
// --- 更新処理 ---
//==================================================================================
void CRanking::Update(void)
{
	CManager *pManager = CManager::GetInstance();
	CInputKeyboard *pKeyboard = pManager->GetInputKeyboard();
	CJoypad *pJoypad = pManager->GetJoypad();

	if ((pKeyboard->GetTrigger(DIK_RETURN)
		|| pJoypad->GetTrigger(CJoypad::KEY_A)
		|| pJoypad->GetTrigger(CJoypad::KEY_START))
		&& pManager->IsTransition() == false)
	{ // 遷移中でなく、Enterが押されたら遷移
		CManager::GetInstance()->GetSound()->Play(CSound::LABEL_SE_BITE);
		pManager->SetTransition(MODE_TITLE);
	}
}

//==================================================================================
// --- 描画処理 ---
//==================================================================================
void CRanking::Draw(void)
{
}

//==================================================================================
// --- 開始時処理 ---
//==================================================================================
void CRanking::Start(void)
{ // カメラ設置
	CCamera::Create(VECTOR3_ONE, VECTOR3_NULL);

	CObject2D::Info info = {};

	info.pos = SCREEN_MIDDLE;
	info.size = SCREEN_SIZE;

	// 白半透明背景生成
	CObject2D::Create(info)->SetColor(Color(1.0f, 1.0f, 1.0f, 0.5f));

	// バックの食べ物出現
	CFoodBackground::Create();

	// ランキングを表示
	CreateRanking();

	// BGM再生
	CManager::GetInstance()->GetSound()->Play(CSound::LABEL_BGM_RESULT);
}

//==================================================================================
// --- ランキング生成処理 ---
//==================================================================================
void CRanking::CreateRanking(void)
{ // ランキングをサーバーから取得
	std::array<int, MAX_RANKING> aScore = {};		// 取得したスコア
	auto pRankingClient = std::make_unique<CRankingClient>();
	if (SUCCEEDED(pRankingClient->Init(SERVER_IPADDRESS, SERVER_PORT)))
	{ // 初期化成功時のみ取得
		aScore = pRankingClient->Get();
		pRankingClient->Uninit();
	}

	int nIdxTexture = CTexture::GetInstance()->Register(NUMBER_TEXTURE);		// 数値のテクスチャインデックス
	float fLeft = SCREEN_MIDDLE.x - (RANK_SIZE.x + RANK_SPACE + SCORE_SIZE.x) * 0.5f;		// 行の左端

	for (int nCntRank = 0; nCntRank < MAX_RANKING; nCntRank++)
	{
		float fPosY = TOP_POS_Y + LINE_SPACE * nCntRank;		// 行のY座標

		// 順位を生成
		CScore::Create(nIdxTexture,
			Vector3(fLeft, fPosY, 0.0f),
			RANK_SIZE,
			RANK_DIGIT,
			nCntRank + 1);

		// スコアを生成
		CScore::Create(nIdxTexture,
			Vector3(fLeft + RANK_SIZE.x + RANK_SPACE, fPosY, 0.0f),
			SCORE_SIZE,
			SCORE_DIGIT,
			aScore[nCntRank]);
	}
}
