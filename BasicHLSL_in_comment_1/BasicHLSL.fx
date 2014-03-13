//--------------------------------------------------------------------------------------
// File: BasicHLSL.fx
//
// The effect file for the BasicHLSL sample.  
// 
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// ���� ����
//--------------------------------------------------------------------------------------
float4 g_MaterialAmbientColor;      // Material's ambient color
float4 g_MaterialDiffuseColor;      // Material's diffuse color
int g_nNumLights;

float3 g_LightDir[3];               // Light's direction in world space
float4 g_LightDiffuse[3];           // Light's diffuse color
float4 g_LightAmbient;              // Light's ambient color

texture g_MeshTexture;              // Color texture for mesh

float    g_fTime;                   // App's time in seconds
float4x4 g_mWorld;                  // World matrix for object
float4x4 g_mWorldViewProjection;    // World * View * Projection matrix



//--------------------------------------------------------------------------------------
// Texture samplers
// �ؽ�ó �ְ� �ش� �ؽ�ó ���� �ɼ� ����
//--------------------------------------------------------------------------------------

sampler MeshTextureSampler = 
sampler_state
{
    Texture = <g_MeshTexture>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
};



//--------------------------------------------------------------------------------------
// shader���� ó���ϴ� �� vertex ���� ����
// CUSTOM Vertex �����ϴ� struct ����
//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Position   : POSITION;   // vertex position 
    float4 Diffuse    : COLOR0;     // vertex diffuse color (note that COLOR0 is clamped from 0..1)
    float2 TextureUV  : TEXCOORD0;  // vertex texture coords 
};


//--------------------------------------------------------------------------------------
// ���� vertex�� �����ϴ� main �Լ�
// ȭ�鿡 �׷����� �� vertex�� ������ �ϴ� ��
//--------------------------------------------------------------------------------------
VS_OUTPUT RenderSceneVS( float4 vPos : POSITION, 
                         float3 vNormal : NORMAL,
                         float2 vTexCoord0 : TEXCOORD0,
                         uniform int nNumLights,
                         uniform bool bTexture,
                         uniform bool bAnimate )
{
    VS_OUTPUT Output;
    float3 vNormalWorldSpace;
  
    float4 vAnimatedPos = vPos;
    
    //������ ���θ� cpu�� ���� �޾ƿ���
	//������ ���� postion�� ����ϴ� ��
	//���⼭ �������� ����� ������ ���� �ǹ�
	if ( bAnimate )
		//vAnimatedPos = vAnimatedPos;
		vAnimatedPos += float4(vNormal, 0) * (sin(g_fTime+5.5)+0.5)*5;
    
    //mul = ����
	//ȭ�� ���� ��Ʈ������ ������ ������ �� ��
    Output.Position = mul(vAnimatedPos, g_mWorldViewProjection);
    
    //��ȭ ���� ���� normal vector ���� ����ؼ� ����
    vNormalWorldSpace = normalize(mul(vNormal, (float3x3)g_mWorld)); // normal (world space)
    
    //���� �ʱ�ȭ
    float3 vTotalLightDiffuse = float3(0,0,0);
    
	//���� ���� -�� ���� �ʵ��� ����
	//if���� �Ⱦ��� ���� max�� ����߳׿�
	for(int i=0; i<nNumLights; i++ )
        vTotalLightDiffuse += g_LightDiffuse[i] * max(0,dot(vNormalWorldSpace, g_LightDir[i]));
    
	//���� ������ light���� rgb ����
    Output.Diffuse.rgb = g_MaterialDiffuseColor * vTotalLightDiffuse + 
                         g_MaterialAmbientColor * g_LightAmbient;   
    
	//���İ��� 1.0���� ����
	Output.Diffuse.a = 1.0f; 
    
    //���ڷ� ���� texture uv ��ǥ �׳� �����ϴ� ��
	//texture�� ������ ����
    if( bTexture ) 
        Output.TextureUV = vTexCoord0; 
    else
        Output.TextureUV = 0; 
    
    return Output;    
}


//--------------------------------------------------------------------------------------
// �ȼ� shader output ����ü
// ���� ���� �� �ۿ� ����. ��ȯ�� �ִ� �͵� �ƴϰ� ������ ��ҿ� � ���� �������� Ȯ��
//--------------------------------------------------------------------------------------
struct PS_OUTPUT
{
    float4 RGBColor : COLOR0;  // Pixel color    
};


//--------------------------------------------------------------------------------------
// �� �ȼ� ������ �ϴ� �Լ�
//--------------------------------------------------------------------------------------
PS_OUTPUT RenderScenePS( VS_OUTPUT In,
                         uniform bool bTexture ) 
{ 
    PS_OUTPUT Output;

	//���÷��� Ȯ��, ��� ���� ���� ���� ���� �ؽ�ó�� vertex�� �ٿ� ��
    //�ռ� vertexShader���� ����� ���� �� ����
    if( bTexture )
        Output.RGBColor = tex2D(MeshTextureSampler, In.TextureUV) * In.Diffuse;
    else
        Output.RGBColor = In.Diffuse;

    return Output;
}


//--------------------------------------------------------------------------------------
// �� ��ü�� �������� �ش� ��ü�� �ؾߵǴ� ���� ������ �� ��
//--------------------------------------------------------------------------------------
technique RenderSceneWithTexture1Light
{
    pass P0
    {          
        VertexShader = compile vs_2_0 RenderSceneVS( 1, true, true );
        PixelShader  = compile ps_2_0 RenderScenePS( true ); // trivial pixel shader (could use FF instead if desired)
    }
}

technique RenderSceneWithTexture2Light
{
    pass P0
    {          
        VertexShader = compile vs_2_0 RenderSceneVS( 2, true, true );
        PixelShader  = compile ps_2_0 RenderScenePS( true ); // trivial pixel shader (could use FF instead if desired)
    }
}

technique RenderSceneWithTexture3Light
{
    pass P0
    {          
        VertexShader = compile vs_2_0 RenderSceneVS( 3, true, true );
        PixelShader  = compile ps_2_0 RenderScenePS( true ); // trivial pixel shader (could use FF instead if desired)
    }
}

technique RenderSceneNoTexture
{
    pass P0
    {          
        VertexShader = compile vs_2_0 RenderSceneVS( 1, false, false );
        PixelShader  = compile ps_2_0 RenderScenePS( false ); // trivial pixel shader (could use FF instead if desired)
    }
}
