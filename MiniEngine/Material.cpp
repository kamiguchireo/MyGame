#include "stdafx.h"
#include "Material.h"

//ルートシグネチャとパイプラインステート周りはカリカリカリ
enum {
	enDescriptorHeap_CB,
	enDescriptorHeap_SRV,
	enNumDescriptorHeap
};
	
void Material::InitTexture(const TkmFile::SMaterial& tkmMat)
{
	if (tkmMat.albedoMap != nullptr) {
		m_albedoMap.InitFromMemory(tkmMat.albedoMap.get(), tkmMat.albedoMapSize);
	}
	
	if (tkmMat.normalMap != nullptr) {
		m_normalMap.InitFromMemory(tkmMat.normalMap.get(), tkmMat.normalMapSize);
	}
	
	if (tkmMat.specularMap != nullptr) {
		m_specularMap.InitFromMemory(tkmMat.specularMap.get(), tkmMat.specularMapSize);
	}
	
	if (tkmMat.reflectionMap != nullptr) {
		m_reflectionMap.InitFromMemory(tkmMat.reflectionMap.get(), tkmMat.reflectionMapSize);
	}
	

	if (tkmMat.refractionMap != nullptr) {
		m_refractionMap.InitFromMemory(tkmMat.refractionMap.get(), tkmMat.refractionMapSize);
	}

}
void Material::InitFromTkmMaterila(
	const TkmFile::SMaterial& tkmMat,
	const wchar_t* fxFilePath,
	const char* vsEntryPointFunc,
	const char* psEntryPointFunc)
{
	//テクスチャをロード。
	InitTexture(tkmMat);
	
	//定数バッファを作成。
	SMaterialParam matParam;
	matParam.hasNormalMap = m_normalMap.IsValid() ? 1 : 0;
	matParam.hasSpecMap = m_specularMap.IsValid() ? 1 : 0;
	m_constantBuffer.Init(sizeof(SMaterialParam), &matParam);

	//ルートシグネチャを初期化。
	//UV座標外のテクスチャは引き延ばし
	m_rootSignature.Init(
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

	if (wcslen(fxFilePath) > 0) {
		//シェーダーを初期化。
		InitShaders(fxFilePath, vsEntryPointFunc, psEntryPointFunc);
		//パイプラインステートを初期化。
		InitPipelineState();
	}
}
void Material::InitPipelineState()
{
	// 頂点レイアウトを定義する。
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 56, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 72, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};


	//パイプラインステートを作成。
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = { 0 };
	psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_vsSkinModel.GetCompiledBlob());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(m_psModel.GetCompiledBlob());
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = TRUE;
	psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 5;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;		//アルベドカラー出力用。
#ifdef SAMPE_16_02
	psoDesc.RTVFormats[1] = DXGI_FORMAT_R16G16B16A16_FLOAT;	//法線出力用。	
	psoDesc.RTVFormats[2] = DXGI_FORMAT_R32_FLOAT;						//Z値。
#else
	psoDesc.RTVFormats[1] = DXGI_FORMAT_R32G32B32A32_FLOAT;			//法線出力用。	
	psoDesc.RTVFormats[2] = DXGI_FORMAT_R8G8B8A8_UNORM;	//Z値。
	psoDesc.RTVFormats[3] = DXGI_FORMAT_R32G32B32A32_FLOAT;		//ワールド座標用
	psoDesc.RTVFormats[4] = DXGI_FORMAT_R8G8B8A8_UNORM;			//スペキュラマップ用	

#endif
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	m_skinModelPipelineState.Init(psoDesc);
	
	//スキンありシャドウマップ用	
	//psoDesc.InputLayout = { inputElementShadowDescs, _countof(inputElementShadowDescs) };
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_vsshadowSkinModel.GetCompiledBlob());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(m_psshadowModel.GetCompiledBlob());
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R32_FLOAT;		//アルベドカラー出力用。
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	m_ShadowSkinModelPipelineState.Init(psoDesc);
	//スキンなしシャドウマップインスタンシング用
	//psoDesc.InputLayout = { inputElementShadowDescs, _countof(inputElementShadowDescs) };
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_vsshadowModelInstancing.GetCompiledBlob());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(m_psshadowModel.GetCompiledBlob());
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	m_ShadownonSkinModelPipelineStateInstancing.Init(psoDesc);
	//スキンなしシャドウマップ用
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_vsshadowModel.GetCompiledBlob());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(m_psshadowModel.GetCompiledBlob());
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	m_ShadownonSkinModelPipelineState.Init(psoDesc);
	
	////続いてスキンなしモデル用を作成。
	////psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
	//psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;		//アルベドカラー出力用。
	//psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_vsNonSkinModel.GetCompiledBlob());
	//psoDesc.PS = CD3DX12_SHADER_BYTECODE(m_psModel.GetCompiledBlob());
	//psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	//m_nonSkinModelPipelineState.Init(psoDesc);

	//続いて半透明マテリアル用。
	//psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;		//アルベドカラー出力用。

	psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_vsSkinModel.GetCompiledBlob());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(m_psModel.GetCompiledBlob());
	psoDesc.BlendState.IndependentBlendEnable = TRUE;
	psoDesc.BlendState.RenderTarget[0].BlendEnable = TRUE;
	psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	psoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;

	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	m_transSkinModelPipelineState.Init(psoDesc);


	psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_vsNonSkinModel.GetCompiledBlob());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(m_psModel.GetCompiledBlob());
	m_transNonSkinModelPipelineState.Init(psoDesc);


}

void Material::InitShaders(
	const wchar_t* fxFilePath,
	const char* vsEntryPointFunc,
	const char* psEntryPointFunc
)
{
	m_vsNonSkinModel.LoadVS(fxFilePath, vsEntryPointFunc);
	m_vsSkinModel.LoadVS(fxFilePath, vsEntryPointFunc);
	m_psModel.LoadPS(fxFilePath, psEntryPointFunc);
	m_vsshadowModelInstancing.LoadVS(L"Assets/shader/ShadowMap.fx", "VSMain_ShadowMapInstancing");
	m_vsshadowModel.LoadVS(L"Assets/shader/ShadowMap.fx", "VSMain_ShadowMap");
	m_vsshadowSkinModel.LoadVS(L"Assets/shader/ShadowMap.fx", "VSMain_ShadowMapSkin");
	m_psshadowModel.LoadPS(L"Assets/shader/ShadowMap.fx", "PSMain_ShadowMap");
}

void Material::BeginRender(RenderContext& rc, int hasSkin,int renderMode,int instanceNum)
{
	rc.SetRootSignature(m_rootSignature);
	
	if (renderMode == enRenderMode_Normal)
	{
		if (hasSkin) {
			//	rc.SetPipelineState(m_skinModelPipelineState);
			rc.SetPipelineState(m_transSkinModelPipelineState);
		}
		else {
			//	rc.SetPipelineState(m_nonSkinModelPipelineState);
			rc.SetPipelineState(m_transNonSkinModelPipelineState);
		}
	}
	else if (renderMode == enRenderMode_CreateShadowMap)
	{
		if (hasSkin) {
			//スキンありシャドウマップ用のパイプラインステート
			rc.SetPipelineState(m_ShadowSkinModelPipelineState);
		}
		else if(instanceNum != 1){
			//スキンなしシャドウマップ用のパイプラインステート
			rc.SetPipelineState(m_ShadownonSkinModelPipelineStateInstancing);
		}
		else
		{
			rc.SetPipelineState(m_ShadownonSkinModelPipelineState);
		}

	}
}
