//==================================================================================
// 
// オブジェクトXの残像クラスのヘッダーファイル [objectXAfterImage.h]
// Author : TENMA SAITO
// 
//==================================================================================
//**********************************************************************************
// *** インクルードファイル ***
//**********************************************************************************
#include "objectXAfterImage.h"
#include "xfile.h"
#include "manager.h"
#include "renderer.h"
#include "texture.h"
#include "matrix.h"

//==================================================================================
// --- 生成処理 ---
//==================================================================================
CObjectXAfterImage *CObjectXAfterImage::Create(std::string_view pFilename,
	const Matrix &mtxWorld,
	const int nLife,
	const float fStartAlpha)
{
	CObjectXAfterImage *pObjectXAfterImage = new CObjectXAfterImage;		// 生成したオブジェクトへのポインタ
	if (pObjectXAfterImage != nullptr)
	{ // 初期化処理
		pObjectXAfterImage->Init(pFilename, mtxWorld, nLife, fStartAlpha);
	}

	return pObjectXAfterImage;
}

//==================================================================================
// --- コンストラクタ ---
//==================================================================================
CObjectXAfterImage::CObjectXAfterImage(const int nPriority) : CObject(nPriority)
{ // タイプの設定
	SetType(TYPE_OBJ_X_AFTERIMAGE);
}

//==================================================================================
// --- デストラクタ ---
//==================================================================================
CObjectXAfterImage::~CObjectXAfterImage()
{
}

//==================================================================================
// --- 初期化処理 ---
//==================================================================================
HRESULT CObjectXAfterImage::Init(std::string_view pFilename,
	const Matrix &mtxWorld,
	const int nLife,
	const float fStartAlpha)
{ // 引数保存
	m_mtxWorld = mtxWorld;
	m_fAlpha = fStartAlpha;
	m_nLife = nLife;

	// 一フレーム当たりのα値の減少値を取得
	m_fDecrease = m_fAlpha / m_nLife;

	// XFILE読み込み
	m_uIdxXInfo = CXFile::GetInstance()->Register(pFilename);

	return (m_uIdxXInfo == INVALID_XFILE_ID) ? E_FAIL : S_OK;
}

//==================================================================================
// --- 終了処理 ---
//==================================================================================
void CObjectXAfterImage::Uninit(void)
{ // 親オブジェクト解放
	CObject::Release();
}

//==================================================================================
// --- 更新処理 ---
//==================================================================================
void CObjectXAfterImage::Update(void)
{
	m_fAlpha -= m_fDecrease;
	m_nLife--;
	if (m_nLife <= 0)
	{ // 体力が0以下になった場合、終了
		Uninit();
	}
}

//==================================================================================
// --- 描画処理 ---
//==================================================================================
void CObjectXAfterImage::Draw(void)
{
	CManager *pManager = CManager::GetInstance();			// マネージャーへのポインタ
	CRenderer *pRenderer = pManager->GetRenderer();			// レンダラーへのポインタ
	LPDIRECT3DDEVICE9 pDevice = pRenderer->GetDevice();		// デバイスへのポインタ
	CTexture *pTexture = CTexture::GetInstance();			// テクスチャへのポインタ
	CXFile *pXFile = CXFile::GetInstance();					// XFILEへのポインタ
	D3DMATERIAL9 matDef;				// 現在のマテリアル保存用
	CXFile::CXInfo *pInfo = nullptr;	// XFILE情報へのポインタ
	Matrix mtxWorld = m_mtxWorld;		// ワールドマトリックス

	//描画するためにXFILE情報を取得
	pInfo = pXFile->GetAddress(m_uIdxXInfo);
	if (pInfo == nullptr) return;		// 取得に失敗した場合、描画スキップ

	// 親マトリックスを適用
	Mtx::CalcParent(&mtxWorld, m_pMtxParent);

	//  ワールドマトリックスの設定
	pDevice->SetTransform(D3DTS_WORLD, &mtxWorld);

	//  現在のマテリアルを保存
	pDevice->GetMaterial(&matDef);

	// 各マテリアルを描画
	for (int nCntMat = 0; nCntMat < static_cast<int>(pInfo->GetMatrialNum()); nCntMat++)
	{
		// マテリアルを取得
		D3DMATERIAL9 mat = *pInfo->GetMaterial(nCntMat);
		mat.Diffuse.a = m_fAlpha;		// マテリアルのα値を下げる

		// マテリアルを設定
		pDevice->SetMaterial(&mat);

		// テクスチャの設定
		pDevice->SetTexture(0, pTexture->GetAddress(pInfo->GetTextureIdx(nCntMat)));

		// モデル(パーツ)の描画
		pInfo->DrawSubset(nCntMat);
	}

	// 保存していたマテリアルを戻す
	pDevice->SetMaterial(&matDef);
}