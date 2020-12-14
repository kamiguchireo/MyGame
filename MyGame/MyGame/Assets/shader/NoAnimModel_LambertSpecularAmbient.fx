//ランバート拡散反射サンプル00。
//拡散反射光のみを確認するためのサンプルです。


static const int NUM_DIRECTIONAL_LIGHT = 4;	//ディレクションライトの本数。
static const int NUM_SHADOW_MAP = 3;
static const float PI = 3.14159265358979323846;

//モデル用の定数バッファ
cbuffer ModelCb : register(b0){
	float4x4 mWorld;
	float4x4 mView;
	float4x4 mProj;
	int IsShadowReciever;
};

//ディレクションライト。
struct DirectionalLight{
	float3 direction;	//ライトの方向。
	float4 color;		//ライトの色。
};
//ライト用の定数バッファ。
cbuffer LightCb : register(b1){
	DirectionalLight directionalLight[NUM_DIRECTIONAL_LIGHT];
	float3 eyePos;					//カメラの視点。
	float specPow;					//スペキュラの絞り。
	float3 ambinentLight;			//環境光。
};

//シャドウマップ用の定数バッファ
cbuffer ShadowCb : register(b2) {
	float4x4 mLVP[NUM_SHADOW_MAP];		//ライトビュープロジェクション行列。
	float3  shadowAreaDepthInViewSpace;	//カメラ空間での影を落とすエリアの深度テーブル。
};


//頂点シェーダーへの入力。
struct SVSIn{
	float4 pos 		: POSITION;		//モデルの頂点座標。
	float3 normal	: NORMAL;		//法線。
	float2 uv 		: TEXCOORD0;	//UV座標。
	float3 tangent	: TANGENT;			//接ベクトル
	float3 biNormal : BINORMAL;
};

//スキンありモデルの頂点シェーダーへの入力
struct SVSInSkin {
	float4 pos		: POSITION;			//モデルの頂点座標
	float3 normal	: NORMAL;			//法線
	float2 uv		: TEXCOORD0;		//UV座標
	float3 tangent	: TANGENT;			//接ベクトル
	uint4 Indices	: BLENDINDICES0;	//この頂点に関連付けされているボーン番号
	float4 Weights	: BLENDWEIGHT0;		//この頂点に関連付けされているボーンウェイト
	float4 biNormal : BINORMAL;
};
//ピクセルシェーダーへの入力。
struct SPSIn{
	float4 pos 			: SV_POSITION;	//スクリーン空間でのピクセルの座標。
	float3 normal		: NORMAL;		//法線。
	float2 uv 			: TEXCOORD0;	//uv座標。
	float3 worldPos		: TEXCOORD1;	//ワールド空間でのピクセルの座標。
	float3 tangent		: TANGENT;		//接ベクトル
	float4 posInview	: TEXCOORD2;
	float3 biNormal		: BINORMAL;
};

//シャドウマップ用のピクセルシェーダーへの入力構造体
struct PSInput_ShadowMap {
	float4 Position:SV_POSITION;		//座標
};

//ピクセルシェーダーからの出力
struct SPSOUT{
	float4 albedo :	SV_Target0;		//アルベド
	float4 normal : SV_Target1;		//法線
	float4 shadow : SV_Target2;		//シャドウ用
	float4 worldPos : SV_Target3;		//ワールド座標
	float4 specularMap : SV_Target4;	//スペキュラマップ
};
//モデルテクスチャ。
Texture2D<float4> g_texture : register(t0);	
Texture2D<float4> g_normalMap : register(t1);
Texture2D<float4> g_specularMap : register(t2);
//ボーン行列
StructuredBuffer<float4x4> boneMatrix : register(t3);
//シャドウテクスチャ
Texture2D<float4>g_shadowMap0:register(t4);
//シャドウテクスチャ
Texture2D<float4>g_shadowMap1:register(t5);
//シャドウテクスチャ
Texture2D<float4>g_shadowMap2:register(t6);
//インスタンシング描画用
StructuredBuffer<float4x4> instanceMatrix : register(t7);

//サンプラステート。
sampler g_sampler : register(s0);

SPSIn VSMainCore(SVSIn vsIn, float4x4 worldMat)
{
	SPSIn psIn = (SPSIn)0;

	psIn.pos = mul(worldMat, vsIn.pos);						//モデルの頂点をワールド座標系に変換。
	psIn.worldPos = psIn.pos.xyz;
	psIn.pos = mul(mView, psIn.pos);						//ワールド座標系からカメラ座標系に変換。
	psIn.posInview = psIn.pos;
	psIn.pos = mul(mProj, psIn.pos);						//カメラ座標系からスクリーン座標系に変換。
	psIn.normal = normalize(mul(mWorld, vsIn.normal));		//法線をワールド座標系に変換。
	psIn.uv = vsIn.uv;

	return psIn;

}

/// <summary>
/// モデル用の頂点シェーダーのエントリーポイント。
/// </summary>
SPSIn VSMain(SVSIn vsIn)
{
	return VSMainCore(vsIn, mWorld);
}

/// <summary>
/// モデル用の頂点シェーダーのエントリーポイント。
/// </summary>
SPSIn VSMainInstancing(SVSIn vsIn,uint instanceID : SV_InstanceID)
{
	return VSMainCore(vsIn, instanceMatrix[instanceID]);
}

//スキン行列を計算
float4x4 CalcSkinMatrix(SVSInSkin vsIn)
{
	float4x4 skinning = 0;
	float4 pos = 0;

	float w = 0.0f;
	[unroll]
	for (int i = 0; i < 3; i++)
	{
		//boneMatrixニーボーン行列が設定されている
		//vsIn.indicesは頂点に埋め込まれた、関連しているボーンの番号
		skinning += boneMatrix[vsIn.Indices[i]] * vsIn.Weights[i];
		w += vsIn.Weights[i];
	}
	//最後のボーンを計算する
	skinning += boneMatrix[vsIn.Indices[3]] * (1.0f - w);
	return skinning;
}

//使用するシャドウマップの番号を取得
int GetCascadeIndex(float zInView)
{
	for (int i = 0; i < NUM_SHADOW_MAP; i++) {
		if (zInView <= shadowAreaDepthInViewSpace[i]) {
			return i;
		}
	}
	return -1;
}

float CalcShadowPercent(Texture2D<float4> tex, float2 uv, float depth/*, float bias*/)
{
	float shadow_val = tex.Sample(g_sampler, uv).r;
	if (depth > shadow_val.r + /*bias*/0.0001f) {
		return 1.0f;
	}
	return 0.0f;
}


//シャドウを計算
float CalcShadow(float3 worldPos, float zInView)
{
	float shadow = 0.0f;
	//シャドウレシーバーのフラグが1
	if (IsShadowReciever == 1)
	{

		//影を落とす。
		//使用するシャドウマップの番号を取得する。
		int cascadeIndex = GetCascadeIndex(zInView);

		if (cascadeIndex == -1)
		{
			return 0;
		}
		float4 posInLVP = mul(mLVP[cascadeIndex], float4(worldPos, 1.0f));
		posInLVP.xyz /= posInLVP.w;
		
		////最大深度傾斜を求める
		//float maxDepthSlope = max(abs(ddx(posInLVP.z)), abs(ddy(posInLVP.z)));
		//float bias = 0.0001f;		//固定バイアス
		//float slopeScaleBias = 0.0001f;		//深度傾斜
		//float depthBiasClamp = 0.1f;		//バイアスクランプ値

		//float shadowBias = bias + slopeScaleBias * maxDepthSlope;
		//shadowBias = min(shadowBias, depthBiasClamp);

		//深度値を取得
		float depth = min(posInLVP.z, 1.0f);
		//uv座標に変換。
		float2 shadowMapUV = float2(0.5f, -0.5f) * posInLVP.xy + float2(0.5f, 0.5f);

		if (cascadeIndex == 0) {
			shadow = CalcShadowPercent(g_shadowMap0, shadowMapUV, depth/*, shadowBias*/);
		}
		else if (cascadeIndex == 1) {
			shadow = CalcShadowPercent(g_shadowMap1, shadowMapUV, depth/*, shadowBias*/);
		}
		else if (cascadeIndex == 2) {

			shadow = CalcShadowPercent(g_shadowMap2, shadowMapUV, depth/*, shadowBias*/);
		}

	}

	return shadow;

}

//スキンありモデル用の頂点シェーダー
SPSIn VSMainSkin(SVSInSkin In)
{
	////初期化
	SPSIn psInput = (SPSIn)0;

	//スキン行列を計算
	float4x4 skinning = CalcSkinMatrix(In);
	//ワールド座標、法線、接ベクトルを計算
	float4 pos = mul(skinning, In.pos);
	psInput.worldPos = pos;
	psInput.normal = normalize(mul(skinning, In.normal));
	psInput.tangent = normalize((mul(skinning, In.tangent)));

	pos = mul(mView, pos);		//ワールド座標系からカメラ座標系に変換
	psInput.posInview = pos;
	pos = mul(mProj, pos);		//カメラ座標系からスクリーン座標系に変換

	psInput.pos = pos;
	psInput.uv = In.uv;
	
	//SPSIn psIn;

	//psIn.pos = mul(mWorld, vsIn.pos);						//モデルの頂点をワールド座標系に変換。
	//psIn.worldPos = psIn.pos.xyz;
	//psIn.pos = mul(mView, psIn.pos);						//ワールド座標系からカメラ座標系に変換。
	//psIn.pos = mul(mProj, psIn.pos);						//カメラ座標系からスクリーン座標系に変換。
	//psIn.normal = normalize(mul(mWorld, vsIn.normal));		//法線をワールド座標系に変換。
	//psIn.uv = vsIn.uv;


	return psInput;
}

float Beckmann(float m, float t)
{
	float t2 = t * t;
	float t4 = t * t * t * t;
	float m2 = m * m;
	float D = 1.0f / (4.0f * m2 * t4);
	D *= exp((-1.0f / m2) * (1.0f - t2) / t2);
	return D;
}

float spcFresnel(float f0, float u)
{
	return f0 + (1 - f0) * pow(1 - u, 5);
}

//L		光源に向かうベクトル
//V		視線に向かうベクトル
//N		法線
float BRDF(float3 L, float3 V, float3 N, float metaric)
{
	float microfacet = 0.3f;
	float f0 = metaric;		//鏡面反射の度合い
	bool include_F = 0;
	bool include_G = 0;
	//光源に向かうベクトルと視線に向かうベクトルのハーフベクトルを求める
	float3 H = normalize(L + V);

	float NdotH = max(0.0f, dot(N, H));
	float VdotH = max(0.0f, dot(V, H));
	float NdotL = max(0.0f, dot(N, L));
	float NdotV = max(0.0f, dot(N, V));

	float D = Beckmann(microfacet, NdotH);
	float F = spcFresnel(f0, VdotH);

	float G = min(1.0f, min(2 * NdotH * NdotV / VdotH, 2 * NdotH * NdotL / VdotH));
	float m = PI * NdotV * NdotL;

	return max(F * D * G / m, 0.0f);
}

float SchlickFresnel(float f0, float f90)
{
	return f0 + (f90 - f0);
}
//N		法線
//L		光源に向かうベクトル
//V		視線に向かうベクトル
//roughness		スペキュラ
float3 NormalizedDisneyDiffuse(float3 N, float3 L, float3 V, float roughness)
{
	//光源に向かうベクトルと視線に向かうベクトルのハーフベクトルを求める
	float3 H = normalize(L + V);

	float energyBias = lerp(0.0f, 0.5f, roughness);
	float energyFactor = lerp(1.0f, 1.0f / 1.51f, roughness);
	//光源に向かうベクトルとハーフベクトルがどれだけ似ているかを内積で求める
	float dotLH = saturate(dot(L, H));
	//法線と光源に向かうベクトルがどれだけ似ているかを内積で求める
	float dotNL = saturate(dot(N, L));
	//法線と視線に向かうベクトルがどれだけ似ているかを内積で求める
	float dotNV = saturate(dot(N, V));

	float Fd90 = energyBias + 2.0 * dotLH * dotLH * roughness;

	float FL = SchlickFresnel(Fd90, dotNL);
	float FV = SchlickFresnel(Fd90, dotNV);

	return (FL * FV) / PI;
}

float3 GetNormal(float3 normal, float3 tangent, float3 biNormal, float2 uv)
{

	float3 binSpaceNormal = g_normalMap.Sample(g_sampler, uv).xyz;
	binSpaceNormal = (binSpaceNormal * 2.0f) - 1.0f;

	float3 newNormal = tangent * binSpaceNormal.x + biNormal * binSpaceNormal.y + normal * binSpaceNormal.z;

	return newNormal;
}

/// <summary>
/// モデル用のピクセルシェーダーのエントリーポイント
/// </summary>
float4 PSMain( SPSIn psIn ) : SV_Target0
{
	float3 lig = 0.0f;
	float metaric = g_specularMap.Sample(g_sampler, psIn.uv).a;
	//ライトをあてる物体から視点に向かって伸びるベクトルを計算する。
	float3 eyeToPixel = eyePos - psIn.worldPos;
	eyeToPixel = normalize(eyeToPixel);
	float3 Normal = GetNormal(psIn.normal, psIn.tangent, psIn.biNormal, psIn.uv);

	//////////////////////////////////////////////////////
	// 拡散反射を計算
	//////////////////////////////////////////////////////
	{
		for( int i = 0; i < NUM_DIRECTIONAL_LIGHT; i++){
			float NdotL = saturate(dot(Normal, -directionalLight[i].direction));	//ライトの逆方向と法線で内積を計算する。
			//if( NdotL < 0.0f){	//内積の計算結果はマイナスになるので、if文で判定する。
			//	NdotL = 0.0f;
			//}			
			
			//拡散反射光を求める
			float disneyDiffuse = NormalizedDisneyDiffuse(Normal, -directionalLight[i].direction, eyeToPixel, 1.0f);
			float3 Diffuse = directionalLight[i].color * NdotL * disneyDiffuse / PI;

			//スペキュラ反射を求める
			float3 Spec = BRDF(-directionalLight[i].direction, eyeToPixel, Normal, metaric);
			Spec *= directionalLight[i].color;
			lig += lerp(Diffuse, Spec, metaric);
		}
	}
	
	//////////////////////////////////////////////////////
	// 環境光を計算
	//////////////////////////////////////////////////////
	lig += ambinentLight;
	//環境光による鏡面反射を計算する。
	//光が法線方向から入射していると考えて鏡面反射を計算する。
	lig += BRDF(Normal, eyeToPixel, Normal, metaric) * ambinentLight * metaric;

	float f;
	f = CalcShadow(psIn.worldPos, psIn.posInview.z);
	//線形補完
	if (f == 1.0f)
	{
		lig *= 0.5f;
	}

	float4 texColor = g_texture.Sample(g_sampler, psIn.uv);
	texColor.xyz *= lig; //光をテクスチャカラーに乗算する。
	return float4(texColor.xyz, 1.0f);	
}

//シャドウマップ生成用のスキンなしモデル頂点シェーダー
PSInput_ShadowMap VSMain_ShadowMap(SVSIn In,uint instanceID : SV_InstanceID)
{
	PSInput_ShadowMap psInput = (PSInput_ShadowMap)0;
	float4 pos = mul(instanceMatrix[instanceID], In.pos);
	//pos = mul(mView, pos);
	pos = mul(mProj, pos);
	psInput.Position = pos;
	return psInput;
}

//シャドウマップ生成用のスキンモデル頂点シェーダー
PSInput_ShadowMap VSMain_ShadowMapSkin(SVSInSkin In)
{
	//初期化
	PSInput_ShadowMap psInput = (PSInput_ShadowMap)0;
	///////////////////////////////////////////////////
	//ここからスキニングを行っている箇所。
	//スキン行列を計算。
	///////////////////////////////////////////////////
	float4x4 skinning = 0;
	float4 pos = 0;
	{

		float w = 0.0f;
		for (int i = 0; i < 3; i++)
		{
			//boneMatrixにボーン行列が設定されていて、
			//In.indicesは頂点に埋め込まれた、関連しているボーンの番号。
			//In.weightsは頂点に埋め込まれた、関連しているボーンのウェイト。
			skinning += boneMatrix[In.Indices[i]] * In.Weights[i];
			w += In.Weights[i];
		}
		//最後のボーンを計算する。
		skinning += boneMatrix[In.Indices[3]] * (1.0f - w);
		//頂点座標にスキン行列を乗算して、頂点をワールド空間に変換。
		//mulは乗算命令。
		pos = mul(skinning,In.pos );
	}

	//pos = mul(mView, pos);
	pos = mul(mProj,pos );
	psInput.Position = pos;
	return psInput;
}

//ピクセルシェーダーのエントリ関数
float4 PSMain_ShadowMap(PSInput_ShadowMap In) : SV_Target0
{
	//射影空間でのZ値を返す
	return In.Position.z / In.Position.w;
}

SPSOUT PSDefferdMain(SPSIn psIn)
{
	SPSOUT psOut;
	psOut.albedo = g_texture.Sample(g_sampler, psIn.uv);
	//法線は0～1
	//float3 Normalmap = (psIn.normal / 2.0f) + 0.5f;
	//psOut.normal.xyz = (psIn.normal / 2.0f) + 0.5f;
	float3 Normal = GetNormal(psIn.normal, psIn.tangent, psIn.biNormal, psIn.uv);
	psOut.normal.xyz = Normal.xyz;
	//シャドウ用
	float f = 0.0f;
	f = CalcShadow(psIn.worldPos, psIn.posInview.z);
	psOut.shadow = f;
	psOut.worldPos.xyz = psIn.worldPos;
	float metaric = g_specularMap.Sample(g_sampler, psIn.uv).g;
	psOut.specularMap = float4(metaric, metaric, metaric, 1.0f);
	return psOut;
}