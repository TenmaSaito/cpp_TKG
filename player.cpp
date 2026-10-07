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
#include "renderer.h"
#include "input.h"
#include "joypad.h"
#include "light.h"
#include "debugproc.h"
#include "vec2math.h"
#include "vec3math.h"
#include "matrix.h"
#include "util.h"
#include "texture.h"
#include "camera.h"
#include "field.h"
#include "wall.h"
#include "ray.h"

//**********************************************************************************
// *** 定数宣言 ***
//**********************************************************************************
namespace
{
	constexpr float PLAYER_SPD = 1.0f;				// プレイヤーの移動速度
	constexpr float JUMP_FORCE = 7.0f;				// ジャンプの強さ
	constexpr float RESIST_POW = 0.25f;				// 摩擦
	constexpr float ANGLE_DEST_ACCELE = 0.1f;		// 目標角度への移動速度
	constexpr float GRAVITY = -0.25f;				// 重力
	constexpr float TERMINAL_VELOCITY = -10.0f;		// 重力の終端速度
	constexpr float MODEL_RADIUS = 10.0f;			// モデルの半径
	constexpr float VALUE_ROT = 0.025f;				// 回転速度
	constexpr float MAX_ROT_VALUE = 0.25f;			// 最大回転速度
	constexpr float RESIST_ROT = 0.008f;			// 減速速度
	constexpr float COR_POW = 0.7f;					// 反発係数
	constexpr float SCALE_VALUE = 1.0f;				// 拡大倍率
	constexpr float RAY_LENGTH = 50.0f;				// レイの長さ
	const Vector3 VEC_QUAT = Vector3(1, 0, 0);		// 初期任意軸
	const Color VEC_PLAYER_RAY_COL = Color(0.0f, 1.0f, 0.0f, 1.0f);		// プレイヤーの向きのレイの色
	const Color VEC_QUAT_RAY_COL = Color(1.0f, 0.0f, 0.0f, 1.0f);		// 任意軸のレイの色
	constexpr std::string_view SHADOW_PATH = "data/TEXTURE/effect001.jpg";		// 影のテクスチャパス
}

//==================================================================================
// --- 生成処理 ---
//==================================================================================
CPlayer *CPlayer::Create(const std::string_view pFilename,
	const Vector3 &pos,
	const float fRadius)
{
	CPlayer *pPlayer = new CPlayer;		// 生成したオブジェクトへのポインタ
	if (pPlayer != nullptr)
	{ // 初期化処理
		pPlayer->Init(pFilename, pos, fRadius);
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

//==================================================================================
// --- 初期化処理 ---
//==================================================================================
HRESULT CPlayer::Init(const std::string_view pFilename,
	const Vector3 &pos,
	const float fRadius)
{ // 親クラスの初期化
	CObjectX::Init(pFilename, pos, VECTOR3_NULL);

	// 各メンバ変数を初期化
	m_fRadius = fRadius;
	m_vecAxis = VEC_QUAT;
	m_fValueRot = 0.0f;
	m_fRotSpeed = VALUE_ROT;
	m_fRotResist = RESIST_ROT;
	m_fJump = JUMP_FORCE;
	m_fCor = COR_POW;
	m_fScaling = SCALE_VALUE;
	Mtx::Identity(&m_mtxRot);

	return S_OK;
}

//==================================================================================
// --- 終了処理 ---
//==================================================================================
void CPlayer::Uninit(void)
{ // 親クラスの終了処理
	CObjectX::Uninit();
}

//==================================================================================
// --- 更新処理 ---
//==================================================================================
void CPlayer::Update(void)
{
	if (m_pField == nullptr)
	{ // 床を取得していなかったら取得
		m_pField = static_cast<CField*>(FindAnyObjectByType(FIELD_PRIORITY, TYPE_FIELD));
	}

	// 入力関連処理
	CharactorMovable();
	if (m_fValueRot > MAX_ROT_VALUE)
	{ // 最大速度を超えていたら、修正
		m_fValueRot = MAX_ROT_VALUE;
	}

	// ステータス変更
	StatusChange();

	// 回転軸を求める
	m_vecAxis = Vec2::ToVector3(Vec2::Direction(Util::FixedRotation(GetRotation()->y - HALF_PI)));
	m_vecAxis.z = m_vecAxis.y;
	m_vecAxis.y = 0.0f;

	// 重力を適用
	m_move.y += GRAVITY;
	if (m_move.y < TERMINAL_VELOCITY)
	{ // 終端速度になったらそれを維持
		m_move.y = TERMINAL_VELOCITY;
	}

	// 移動量を計算
	float fCircumference = 0.0f;		// 円周の長さ
	float fPower = 0.0f;				// 移動量
	fCircumference = DOUBLE_PI * GetRadius();				// 円周の長さを求める
	fPower = (m_fValueRot / DOUBLE_PI) * fCircumference;	// 移動量を求める

	// 加速度を更新
	Vector2 move = Vec2::Direction(Util::FixedRotation(-GetRotation()->y)) * fPower;
	m_move.x = move.x;
	m_move.z = -move.y;

	// 位置を更新
	SetPosition(*GetPosition() + m_move);
	if (GetPosition()->y < GetRadius())
	{ // 地面についていれば、フラグリセット
		m_bJump = false;

		// 座標を修正
		SetPosition(Vector3(GetPosition()->x, GetRadius(), GetPosition()->z));
		m_move.y *= -m_fCor;		// 下への加速度を反転し跳ね返りを起こす
	}

	if (m_pField != nullptr)
	{
		Vector4 boader;		// 境界線
		Vector3 pos = *GetPosition();		// 現在位置

		// 境界線を計算
		boader.x = m_pField->GetPosition()->x + (m_pField->GetSize()->x * 0.5f) - GetRadius();		// 右端
		boader.y = m_pField->GetPosition()->y + (m_pField->GetSize()->x * 0.5f) - GetRadius();		// 上端
		boader.z = m_pField->GetPosition()->y - (m_pField->GetSize()->x * 0.5f) + GetRadius();		// 下端
		boader.w = m_pField->GetPosition()->x - (m_pField->GetSize()->x * 0.5f) + GetRadius();		// 左端

		// X軸の判定
		if (pos.x > boader.x) pos.x = boader.x;
		else if (pos.x < boader.w) pos.x = boader.w;
		
		// Z軸の判定
		if (pos.z > boader.y) pos.z = boader.y;
		else if (pos.z < boader.z) pos.z = boader.z;

		// 強制後の位置を反映
		SetPosition(pos);
	}

	// デバッグ用ライン表示
	CRay vecPlayer;		// プレイヤーの向き
	Vector2 vec = Vec2::Direction(Util::FixedRotation(-GetRotation()->y));		// プレイヤーの向き

	// プレイヤーの向きの情報を指定し描画
	vecPlayer.SetStart(*GetPosition());						
	vecPlayer.SetVector(Vector3(vec.x, 0.0f, -vec.y));		
	vecPlayer.SetLength(RAY_LENGTH + GetRadius());
	vecPlayer.SetColor(VEC_PLAYER_RAY_COL);
	vecPlayer.Draw();

	// 任意軸の向きの情報を指定し描画
	vecPlayer.SetVector(m_vecAxis);
	vecPlayer.SetColor(VEC_QUAT_RAY_COL);
	vecPlayer.Draw();

	// 任意軸の逆向きの情報を指定し描画
	vecPlayer.SetVector(-m_vecAxis);
	vecPlayer.Draw();

	// デバッグ情報表示
	CDebugProc *pProc = CManager::GetInstance()->GetDebugProc();		// デバッグ表示へのポインタ
	pProc->Print("====== プレイヤー ======\n");
	pProc->Print("座標     ({:.2f}/{:.2f}/{:.2f})\n", PRINT_VECTOR3(*GetPosition()));
	pProc->Print("角度     ({:.2f}/{:.2f}/{:.2f})\n", PRINT_VECTOR3(*GetRotation()));
	pProc->Print("移動量   ({:.2f}/{:.2f}/{:.2f})\n", PRINT_VECTOR3(m_move));
	pProc->Print("回転軸   ({:.2f}/{:.2f}/{:.2f})\n", PRINT_VECTOR3(m_vecAxis));
	pProc->Print("回転量   ({:.4f})\n", m_fValueRot);
	pProc->Print("回転速度 ({:.4f}) [↑/↓]\n", m_fRotSpeed);
	pProc->Print("減速係数 ({:.4f}) [Y/H]\n", m_fRotResist);
	pProc->Print("跳躍力   ({:.4f}) [R/F]\n", m_fJump);
	pProc->Print("反発係数 ({:.4f}) [T/G]\n", m_fCor);
	pProc->Print("半径     ({:.4f}) [1/2]\n", GetRadius());
}

//==================================================================================
// --- 描画処理 ---
//==================================================================================
void CPlayer::Draw(void)
{ // 親クラスの描画処理
	CManager *pManager = CManager::GetInstance();			// マネージャーへのポインタ
	CRenderer *pRenderer = pManager->GetRenderer();			// レンダラーへのポインタ
	LPDIRECT3DDEVICE9 pDevice = pRenderer->GetDevice();		// デバイスへのポインタ
	Matrix *pMtx = GetMatrixPtr();		// マトリックスへのポインタ
	Matrix mtxWorld;		// ワールドマトリックス
	Matrix mtxShadow;		// シャドウマトリックス

	// マトリックスの初期化
	Mtx::Identity(&mtxWorld);

	// マトリックスのスケーリング
	Mtx::CalcScale(&mtxWorld, m_scl * m_fScaling);

	// 回転軸における指定の回転角からクォータニオンを作成
	D3DXQuaternionRotationAxis(&m_quat, &m_vecAxis, m_fValueRot);

	// クォータニオンから回転マトリックスを作成
	Mtx::CalcRotation(&m_mtxRot, m_quat);

	// 回転マトリックスを掛け合わせる
	D3DXMatrixMultiply(&mtxWorld, &mtxWorld, &m_mtxRot);

	// 移動マトリックスを計算
	Mtx::CalcPosition(&mtxWorld, *GetPosition());

	// 法線ベクトルの再正規化を有効化
	pDevice->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE);

	// 描画
	CObjectX::Draw(mtxWorld);

	// 床のシャドウマトリックスを生成
	Mtx::CreateShadow(&mtxWorld,
		Vector3(0.0f, 0.1f, 0.0f),
		Vector3(0.0f, 1.0f, 0.0f),
		*pManager->GetLight()->GetLight(0),
		&mtxShadow);

	// 影を描画
	CObjectX::DrawShadow(mtxShadow);

	std::deque apWall = FindObjectsByType(CObject::TYPE_WALL);
	for (auto &wall : apWall)
	{
		CWall *pWall = static_cast<CWall*>(wall);
		Vector3 nor = VECTOR3_NULL;

		// 法線を計算
		Vector2 vec = Vec2::Direction(pWall->GetRotation()->y);
		nor.x = -vec.x;
		nor.z = -vec.y;

		// 壁のシャドウマトリックスを生成
		Mtx::CreateShadow(&mtxWorld,
			Vector3(pWall->GetPosition()->x + nor.x, pWall->GetPosition()->y, pWall->GetPosition()->z + nor.z),
			nor,
			*pManager->GetLight()->GetLight(0),
			&mtxShadow);

		// 影を描画
		CObjectX::DrawShadow(mtxShadow);
	}

	// 法線ベクトルの再正規化を無効化
	pDevice->SetRenderState(D3DRS_NORMALIZENORMALS, FALSE);
}

//==================================================================================
// --- 通知処理 ---
//==================================================================================
void CPlayer::Notified(CObject *pObject, std::string_view message)
{
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
	float fRotDest = GetRotation()->y;					// 目標角度
	CDebugProc *pProc = pManager->GetDebugProc();		// デバッグ表示へのポインタ
	CCamera *pPlayerCam = CCamera::GetCamera(CCamera::TYPE_PLAYER);		// プレイヤーカメラへのポインタ
	const Vector3 *pCamRotate = pPlayerCam->GetRotate();				// カメラの角度

	if (pKeyboard->GetPress(DIK_W))
	{ // Wを押したとき
		if (pKeyboard->GetPress(DIK_A))
		{ // Aを押したとき
			// カメラの角度に合わせて、モデルの目標角度を求める！
			fRotDest = pCamRotate->y - D3DX_PI * 1.25f;
			m_rotDest.y = Util::FixedRotation(fRotDest);
		}
		else if (pKeyboard->GetPress(DIK_D))
		{ // Dを押したとき
			// カメラの角度に合わせて、モデルの目標角度を求める！
			fRotDest = pCamRotate->y + (D3DX_PI * 1.25f);
			m_rotDest.y = Util::FixedRotation(fRotDest);
		}
		else
		{ // 純粋なW入力時
			// カメラの角度に合わせて、モデルの目標角度を求める！
			fRotDest = pCamRotate->y + D3DX_PI;
			m_rotDest.y = Util::FixedRotation(fRotDest);
		}

		if (m_fValueRot <= 0.0f)
		{ // 回転速度が0.0f以下の場合、角度を目標角度に強制
			SetRotation(Vector3(0.0f, Util::FixedRotation(m_rotDest.y), 0.0f));
		}

		m_fValueRot += m_fRotSpeed;		// 回転速度を更新
	}
	else if (pKeyboard->GetPress(DIK_S))
	{ // Sを押したとき
		if (pKeyboard->GetPress(DIK_A))
		{ // Aを押したとき
			// カメラの角度に合わせて、モデルの目標角度を求める！
			fRotDest = pCamRotate->y + QUARTER_PI;
			m_rotDest.y = Util::FixedRotation(fRotDest);
		}
		else if (pKeyboard->GetPress(DIK_D))
		{ // Dを押したとき
			// カメラの角度に合わせて、モデルの目標角度を求める！
			fRotDest = pCamRotate->y - QUARTER_PI;
			m_rotDest.y = Util::FixedRotation(fRotDest);
		}
		else
		{ // 純粋なS入力時
			// カメラの角度に合わせて、モデルの目標角度を求める！
			fRotDest = pCamRotate->y;
			m_rotDest.y = Util::FixedRotation(fRotDest);
		}

		if (m_fValueRot <= 0.0f)
		{ // 回転速度が0.0f以下の場合、角度を目標角度に強制
			SetRotation(Vector3(0.0f, Util::FixedRotation(m_rotDest.y), 0.0f));
		}

		m_fValueRot += m_fRotSpeed;		// 回転速度を更新
	}
	else if (pKeyboard->GetPress(DIK_A))
	{ // Aを押したとき
		// カメラの角度に合わせて、モデルの目標角度を求める！
		fRotDest = pCamRotate->y + HALF_PI;
		m_rotDest.y = Util::FixedRotation(fRotDest);
		if (m_fValueRot <= 0.0f)
		{ // 回転速度が0.0f以下の場合、角度を目標角度に強制
			SetRotation(Vector3(0.0f, Util::FixedRotation(m_rotDest.y), 0.0f));
		}

		m_fValueRot += m_fRotSpeed;		// 回転速度を更新
	}
	else if (pKeyboard->GetPress(DIK_D))
	{ // Dを押したとき
		// カメラの角度に合わせて、モデルの目標角度を求める！
		fRotDest = pCamRotate->y - HALF_PI;
		m_rotDest.y = Util::FixedRotation(fRotDest);
		if (m_fValueRot <= 0.0f)
		{ // 回転速度が0.0f以下の場合、角度を目標角度に強制
			SetRotation(Vector3(0.0f, Util::FixedRotation(m_rotDest.y), 0.0f));
		}

		m_fValueRot += m_fRotSpeed;		// 回転速度を更新
	}
	else if (pJoypad->GetStick(CJoypad::STICK_LEFT, &stick) && Vec3::Length(stick) > STICK_DEADZONE)
	{ // ジョイパッドからの入力で一定以上倒されていれば
		float fAngle = atan2f(stick.x, stick.y);		// スティックの倒された角度
		float fSpeed = Vec3::Length(stick);				// 倒された強さ

		// カメラの角度に合わせて、モデルの目標角度を求める！
		fRotDest = (fAngle + D3DX_PI);
		m_rotDest.y = Util::FixedRotation(fRotDest);
		if (m_fValueRot <= 0.0f)
		{ // 回転速度が0.0f以下の場合、角度を目標角度に強制
			SetRotation(Vector3(0.0f, Util::FixedRotation(m_rotDest.y), 0.0f));
		}

		m_fValueRot += m_fRotSpeed;		// 回転速度を更新
	}
	else
	{ // 動いていない時は回転角を減少
		m_fValueRot -= m_fRotResist;
		if (m_fValueRot <= FLT_MIN)
		{ // 回転速度が0.0f以下になった場合、0.0fに戻す
			m_fValueRot = 0.0f;
		}
	}

	if ((pKeyboard->GetTrigger(DIK_SPACE) || pJoypad->GetTrigger(CJoypad::KEY_A))
		&& m_bJump == false)
	{ // ジャンプボタンを押したとき
		m_move.y = m_fJump;		// Y軸の加速度を設定
		m_bJump = true;			// フラグを立てる
	}

	// 角度修正
	fRotDest = Util::FixedRotation(fRotDest);

	// 差分を求める
	float fValue = fRotDest - GetRotation()->y;

	// 差分を正規化して、絶対値に変換
	fValue = fabsf(Util::FixedRotation(fValue));
	if (fValue >= D3DX_PI * 0.75f)
	{ // プレイヤーの向きが前回とπ(+ε)分離れていたら、回転量をマイナスにし、角度即時適用
		m_fValueRot *= -1;

		// 角度更新
		SetRotation(Vector3(0.0f, fRotDest, 0.0f));
	}
	else
	{ // 通常の角度更新
		float fAngle = m_rotDest.y - GetRotation()->y;
		fAngle = Util::FixedRotation(fAngle);

		SetRotation(Vector3(0.0f, Util::FixedRotation(GetRotation()->y + (fAngle * ANGLE_DEST_ACCELE)), 0.0f));
	}
}

//==================================================================================
// --- ステータス変更処理 ---
//==================================================================================
void CPlayer::StatusChange(void)
{
	CManager *pManager = CManager::GetInstance();		// マネージャへのポインタ
	CInputKeyboard *pKeyboard = pManager->GetInputKeyboard();		// キーボードへのポインタ
	int nRepeatWait = 5;		// リピートの待機時間

	// 回転速度
	if (pKeyboard->GetRepeat(DIK_UP, nRepeatWait, 1))
	{ // ↑を押したとき
		m_fRotSpeed += 0.001f;
	}
	else if (pKeyboard->GetRepeat(DIK_DOWN, nRepeatWait, 1))
	{ // ↓を押したとき
		m_fRotSpeed -= 0.001f;
	}

	// 減速係数
	if (pKeyboard->GetRepeat(DIK_Y, nRepeatWait, 1))
	{ // ↑を押したとき
		m_fRotResist += 0.0001f;
	}
	else if (pKeyboard->GetRepeat(DIK_H, nRepeatWait, 1))
	{ // ↓を押したとき
		m_fRotResist -= 0.0001f;
	}

	// 跳躍力
	if (pKeyboard->GetRepeat(DIK_R, nRepeatWait, 1))
	{ // ↑を押したとき
		m_fJump += 0.01f;
	}
	else if (pKeyboard->GetRepeat(DIK_F, nRepeatWait, 1))
	{ // ↓を押したとき
		m_fJump -= 0.01f;
	}

	// 反発係数
	if (pKeyboard->GetRepeat(DIK_T, nRepeatWait, 1))
	{ // ↑を押したとき
		m_fCor += 0.01f;
	}
	else if (pKeyboard->GetRepeat(DIK_G, nRepeatWait, 1))
	{ // ↓を押したとき
		m_fCor -= 0.01f;
	}

	// 拡大倍率
	if (pKeyboard->GetRepeat(DIK_1, nRepeatWait, 1))
	{ // ↑を押したとき
		m_fScaling += 0.1f;
	}
	else if (pKeyboard->GetRepeat(DIK_2, nRepeatWait, 1))
	{ // ↓を押したとき
		m_fScaling -= 0.1f;
	}
}