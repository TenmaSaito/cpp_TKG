//==================================================================================
// 
// プレイヤークラスのソースファイル [player.cpp]
// Author : TENMA SAITO
// Date   : 2026/9/28
// 
//==================================================================================
//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "player.h"
#include "manager.h"
#include "input.h"
#include "joypad.h"
#include "debugproc.h"
#include "shadow.h"
#include "vec3math.h"
#include "util.h"
#include "texture.h"
#include "camera.h"

//**********************************************************************************
// *** 定数宣言 ***
//**********************************************************************************
namespace
{
	constexpr float PLAYER_SPD = 1.0f;		// プレイヤーの移動速度
	constexpr float JUMP_FORCE = 2.5f;		// ジャンプの強さ
	constexpr float RESIST_POW = 0.25f;		// 摩擦
	constexpr float ANGLE_DEST_ACCELE = 0.05f;		// 目標角度への移動速度
	constexpr float GRAVITY = -0.1f;				// 重力
	constexpr float TERMINAL_VELOCITY = -10.0f;		// 重力の終端速度
	constexpr std::string_view SHADOW_PATH = "data/TEXTURE/effect001.jpg";		// 影のテクスチャパス
}

//==================================================================================
// --- 生成処理 ---
//==================================================================================
CPlayer *CPlayer::Create(const char *pFilename, const Vector3 &pos, const Vector3 &rot)
{
	CPlayer *pPlayer = new CPlayer;		// 生成したオブジェクトへのポインタ
	if (pPlayer != nullptr)
	{ // 初期化処理
		pPlayer->Init(pFilename, pos, rot);
	}

	return pPlayer;
}

//==================================================================================
// --- コンストラクタ ---
//==================================================================================
CPlayer::CPlayer(const int nPriority) : CObjectX(nPriority)
{ // タイプの設定
	SetType(TYPE_PLAYER);
}

//==================================================================================
// --- デストラクタ ---
//==================================================================================
CPlayer::~CPlayer()
{
}

#include "filestream.h"

//==================================================================================
// --- 初期化処理 ---
//==================================================================================
HRESULT CPlayer::Init(const char *pFilename,
	const Vector3 &pos,
	const Vector3 &rot)
{ // 親クラスの初期化
	CObjectX::Init(pFilename, pos, rot);

	// 影を作成し、監視者に自分を追加
	m_pShadow = CShadow::Create(pos, rot, Vector2(GetVtxMax()->x - GetVtxMin()->x, GetVtxMax()->z - GetVtxMin()->z));
	m_pShadow->AddObserver(this);

	// 影にテクスチャを割り当て
	m_pShadow->BindTexture(CTexture::GetInstance()->Register(SHADOW_PATH));

	return S_OK;
}

//==================================================================================
// --- 終了処理 ---
//==================================================================================
void CPlayer::Uninit(void)
{ 
	if (m_pShadow != nullptr)
	{ // 影の管理対象から外れる
		m_pShadow->RemoveObserver(this);
		
		// 影を破棄
		m_pShadow->Uninit();
		m_pShadow = nullptr;
	}

	// 親クラスの終了処理
	CObjectX::Uninit();
}

//==================================================================================
// --- 更新処理 ---
//==================================================================================
void CPlayer::Update(void)
{ // 入力関連処理
	CharactorMovable();

	// 位置を更新
	SetPosition(*GetPosition() + m_move);
	if (GetPosition()->y < 0.0f)
	{ // 地面についていれば、フラグリセット
		m_bJump = false;

		// 座標を修正
		SetPosition(Vector3(GetPosition()->x, 0.0f, GetPosition()->z));
	}

	// 角度を更新
	float fAngle = 0.0f;		// 角度修正用変数

	// 角度の修正!
	m_rotDest = Vec3::FixedRotation(m_rotDest);

	// 角度の差を求める!
	fAngle = (m_rotDest.y - GetRotation()->y);
	fAngle = Util::FixedRotation(fAngle);

	// 角度の差分だけ角度を徐々に回転する！
	SetRotation(Vector3(0.0f, Util::FixedRotation(GetRotation()->y + fAngle * ANGLE_DEST_ACCELE), 0.0f));

	// 加速度を更新
	m_move.x += (VECTOR3_NULL.x - m_move.x) * RESIST_POW;
	m_move.z += (VECTOR3_NULL.z - m_move.z) * RESIST_POW;

	// 重力を適用
	m_move.y += GRAVITY;
	if (m_move.y < TERMINAL_VELOCITY)
	{ // 終端速度になったらそれを維持
		m_move.y = TERMINAL_VELOCITY;
	}

	if (m_pShadow != nullptr)
	{ // nullでなければ、影を更新
		m_pShadow->SetPosition(Vector3(GetPosition()->x, 0.01f, GetPosition()->z));
		m_pShadow->SetRotation(*GetRotation());
	}

	// プレイヤーの位置を表示
	CDebugProc *pProc = CManager::GetInstance()->GetDebugProc();		// デバッグ表示へのポインタ
	pProc->Print("Player : POS [{:02f}/{:02f}/{:02f}]\n", PRINT_VECTOR3((*GetPosition())));
	pProc->Print("       : ROT [{:02f}/{:02f}/{:02f}]\n", PRINT_VECTOR3((*GetRotation())));
}

//==================================================================================
// --- 描画処理 ---
//==================================================================================
void CPlayer::Draw(void)
{ // 親クラスの描画処理
	CObjectX::Draw();
}

//==================================================================================
// --- 通知処理 ---
//==================================================================================
void CPlayer::Notified(CObject *pObject, std::string_view message)
{
	if (pObject == m_pShadow && message == NOTIFY_WHEN_DEATH)
	{ // もし自身の管理している影からの通知で、死亡メッセージなら
		m_pShadow = nullptr;		// ポインタを手放す
	}
}

//==================================================================================
// --- 入力関連処理 ---
//==================================================================================
void CPlayer::CharactorMovable(void)
{
	CManager *pManager = CManager::GetInstance();		// マネージャへのポインタ
	CInputKeyboard *pKeyboard = pManager->GetInputKeyboard();		// キーボードへのポインタ
	CJoypad *pJoypad = pManager->GetJoypad();			// ジョイパッドへのポインタ
	Vector3 stick = VECTOR3_NULL;						// ジョイパッドのスティック入力

	if (pKeyboard->GetPress(DIK_W))
	{ // Wを押したとき
		if (pKeyboard->GetPress(DIK_A))
		{ // Aを押したとき
			// カメラの角度に合わせて、平行移動！
			m_move.z += cosf(-QUARTER_PI) * PLAYER_SPD;
			m_move.x += sinf(-QUARTER_PI) * PLAYER_SPD;

			// カメラの角度に合わせて、モデルの目標角度を求める！
			m_rotDest.y = -D3DX_PI * 1.25f;
		}
		else if (pKeyboard->GetPress(DIK_D))
		{ // Dを押したとき
			// カメラの角度に合わせて、平行移動！
			m_move.z += cosf(QUARTER_PI) * PLAYER_SPD;
			m_move.x += sinf(QUARTER_PI) * PLAYER_SPD;

			// カメラの角度に合わせて、モデルの目標角度を求める！
			m_rotDest.y = (D3DX_PI * 1.25f);
		}
		else
		{ // 純粋なW入力時
			// カメラの角度に合わせて、平行移動！
			m_move.x += sinf(0.0f) * PLAYER_SPD;
			m_move.z += cosf(0.0f) * PLAYER_SPD;

			// カメラの角度に合わせて、モデルの目標角度を求める！
			m_rotDest.y = D3DX_PI;
		}
	}
	else if (pKeyboard->GetPress(DIK_S))
	{ // Sを押したとき
		if (pKeyboard->GetPress(DIK_A))
		{ // Aを押したとき
			// カメラの角度に合わせて、平行移動！
			m_move.z += cosf(D3DX_PI * 1.25f) * PLAYER_SPD;
			m_move.x += sinf(D3DX_PI * 1.25f) * PLAYER_SPD;

			// カメラの角度に合わせて、モデルの目標角度を求める！
			m_rotDest.y = QUARTER_PI;
		}
		else if (pKeyboard->GetPress(DIK_D))
		{ // Dを押したとき
			// カメラの角度に合わせて、平行移動！
			m_move.z += cosf(-D3DX_PI * 1.25f) * PLAYER_SPD;
			m_move.x += sinf(-D3DX_PI * 1.25f) * PLAYER_SPD;

			// カメラの角度に合わせて、モデルの目標角度を求める！
			m_rotDest.y = -QUARTER_PI;
		}
		else
		{ // 純粋なS入力時
			// カメラの角度に合わせて、平行移動！
			m_move.x += sinf(D3DX_PI) * PLAYER_SPD;
			m_move.z += cosf(D3DX_PI) * PLAYER_SPD;

			// カメラの角度に合わせて、モデルの目標角度を求める！
			m_rotDest.y = 0.0f;
		}
	}
	else if (pKeyboard->GetPress(DIK_A))
	{ // Aを押したとき
		// カメラの角度に合わせて、平行移動！
		m_move.z += cosf(D3DX_PI * 1.5f) * PLAYER_SPD;
		m_move.x += sinf(D3DX_PI * 1.5f) * PLAYER_SPD;

		// カメラの角度に合わせて、モデルの目標角度を求める！
		m_rotDest.y = HALF_PI;
	}
	else if (pKeyboard->GetPress(DIK_D))
	{ // Dを押したとき
		// カメラの角度に合わせて、平行移動！
		m_move.z += cosf(HALF_PI) * PLAYER_SPD;
		m_move.x += sinf(HALF_PI) * PLAYER_SPD;

		// カメラの角度に合わせて、モデルの目標角度を求める！
		m_rotDest.y = (D3DX_PI * 1.5f);
	}
	else if (pJoypad->GetStick(CJoypad::STICK_LEFT, &stick) && Vec3::Length(stick) > STICK_DEADZONE)
	{ // ジョイパッドからの入力で一定以上倒されていれば
		float fAngle = atan2f(stick.x, stick.y);		// スティックの倒された角度
		float fSpeed = Vec3::Length(stick);				// 倒された強さ

		// カメラの角度に合わせて、平行移動！
		m_move.z += cosf(fAngle) * (PLAYER_SPD * fSpeed);
		m_move.x += sinf(fAngle) * (PLAYER_SPD * fSpeed);

		// カメラの角度に合わせて、モデルの目標角度を求める！
		m_rotDest.y = (fAngle + D3DX_PI);
	}

	if ((pKeyboard->GetTrigger(DIK_SPACE) || pJoypad->GetTrigger(CJoypad::KEY_A))
		&& m_bJump == false)
	{ // ジャンプボタンを押したとき
		m_move.y = JUMP_FORCE;		// Y軸の加速度を設定
		m_bJump = true;				// フラグを立てる
	}
}