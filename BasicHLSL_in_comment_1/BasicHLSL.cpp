//--------------------------------------------------------------------------------------
// File: BasicHLSL.cpp
//
// This sample shows a simple example of the Microsoft Direct3D's High-Level 
// Shader Language (HLSL) using the Effect interface. 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "DXUT.h"
#include "DXUTcamera.h"
#include "SDKmisc.h"
#include "resource.h"


//--------------------------------------------------------------------------------------
// �۷ι� ����
//--------------------------------------------------------------------------------------
CModelViewerCamera          g_Camera;               // ī�޶� �� (������ ���� ��)
ID3DXEffect*                g_pEffect = NULL;       // 
ID3DXMesh*                  g_pMesh = NULL;         // Mesh object
IDirect3DTexture9*          g_pMeshTexture = NULL;  // Mesh texture

//����ȭ �ɼ�
bool                        g_bEnablePreshader;     // if TRUE, then D3DXSHADER_NO_PRESHADER is used when compiling the shader

D3DXMATRIXA16               g_mCenterWorld; //���� ��Ʈ����

//�ִ� ���� ���� 3��
#define MAX_LIGHTS 3
//���� ���� ������ Warpping �Ǿ� �ִ� ��ü
//3d3 ���� ��ü(device) � ���� ���ԵǾ� ����
//��, ���⼭ �ٷ� ���Ǵ� ���� �ƴ�
CDXUTDirectionWidget g_LightControl[MAX_LIGHTS];

//���� ���
float                       g_fLightScale;
//Ȱ�� ���� ����
int                         g_nNumActiveLights;
//���� Ȱ��ȭ �� ����
int                         g_nActiveLight;


//--------------------------------------------------------------------------------------
// Forward declarations 
// �����Ϸ����� �Լ� ����Ʈ ����
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
                                 void* pUserContext );
HRESULT CALLBACK OnResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
                                void* pUserContext );
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext );
void CALLBACK OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext );
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext );
void CALLBACK OnLostDevice( void* pUserContext );
void CALLBACK OnDestroyDevice( void* pUserContext );

void InitApp();
HRESULT LoadMesh( IDirect3DDevice9* pd3dDevice, WCHAR* strFileName, ID3DXMesh** ppMesh );


// main �Լ� ��
INT WINAPI wWinMain( HINSTANCE, HINSTANCE, LPWSTR, int )
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    DXUTSetCallbackD3D9DeviceCreated( OnCreateDevice );
    DXUTSetCallbackD3D9DeviceReset( OnResetDevice );
    DXUTSetCallbackD3D9FrameRender( OnFrameRender );
	
	//focus�� ������� ��. �� â���/��üȭ�� ��� ����� ó�� ����
    DXUTSetCallbackD3D9DeviceLost( OnLostDevice );
    DXUTSetCallbackD3D9DeviceDestroyed( OnDestroyDevice );
    DXUTSetCallbackMsgProc( MsgProc );

	//update ����, ���� ������ ������� �����ϴ� �κ�
	DXUTSetCallbackFrameMove( OnFrameMove );

	//���� �ʱ�ȭ
    InitApp();

	//Ű���� ����Ű�� �޾� ó���ϵ��� �ϴ� �÷��� ���� �Լ�
	DXUTSetHotkeyHandling( true, true, true );

    DXUTCreateWindow( L"BasicHLSL" );
    DXUTCreateDevice( true, 640, 480 );

	// main ���� ����
	// ���� ���� �� ���õǴ� ������ ������ ȭ�鿡 ǥ����
    DXUTMainLoop();


    return DXUTGetExitCode();
}


//--------------------------------------------------------------------------------------
// Initialize the app 
//--------------------------------------------------------------------------------------
void InitApp()
{
    //�����Ϸ����� ���̴� ��� ���θ� �̸� �˸�
	//�� ȿ������ �����Ϸ� �������� ����
	g_bEnablePreshader = true;

	//�� ������ �ʱ� �� ����
    for( int i = 0; i < MAX_LIGHTS; i++ )
        g_LightControl[i].SetLightDirection( D3DXVECTOR3( sinf( D3DX_PI * 2 * i / MAX_LIGHTS - D3DX_PI / 6 ),
                                                          0, -cosf( D3DX_PI * 2 * i / MAX_LIGHTS - D3DX_PI / 6 ) ) );

	//�ʱ� Ȱ��ȭ �� ���� idx ��ȣ
    g_nActiveLight = 0;

	//�ʱ� Ȱ��ȭ�� ���� ����
    g_nNumActiveLights = 1;
    
	//���� ��� ��
	g_fLightScale = 1.0f;

}


//--------------------------------------------------------------------------------------
// ����̽� ���� �Լ�
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
                                 void* pUserContext )
{
    HRESULT hr;

    //mesh �ҷ��ͼ� g_pMesh�� ����
    V_RETURN( LoadMesh( pd3dDevice, L"tiny\\tiny.x", &g_pMesh ) );

    D3DXVECTOR3* pData;
    D3DXVECTOR3 vCenter;
    FLOAT fObjectRadius;

	//�޽��� �� buffer�� ���� lock
	//lock�� ���ؽ� ������ ��ġ ���� �ҷ���(��� �ִ��� ��� ���� ����)
	//ù ��° ���ڰ� ���� �ɼ�(3= readonly)
    V( g_pMesh->LockVertexBuffer( 3, ( LPVOID* )&pData ) );
	//�޽� ��谪 ���� = ��� vertex ��ȸ
	//��� vertex��ȸ �� ��� ���= ����, ���� �հŸ��� vertex�� ������
    V( D3DXComputeBoundingSphere( pData, g_pMesh->GetNumVertices(),
                                 D3DXGetFVFVertexSize( g_pMesh->GetFVF() ), &vCenter, &fObjectRadius ) );
	//buffer unlock
    V( g_pMesh->UnlockVertexBuffer() );


	//���� ��ǥ �ʱ� ����
	//mesh���� ���� �߾Ӱ��� �������� ����
    D3DXMatrixTranslation( &g_mCenterWorld, -vCenter.x, -vCenter.y, -vCenter.z );
    D3DXMATRIXA16 m;
    D3DXMatrixRotationY( &m, D3DX_PI );
    g_mCenterWorld *= m;
    D3DXMatrixRotationX( &m, D3DX_PI / 2.0f );
    g_mCenterWorld *= m;


	//warping�� device��ü ����
    V_RETURN( CDXUTDirectionWidget::StaticOnD3D9CreateDevice( pd3dDevice ) );

	//���� �̵� ���°� ���� �̷�� �ִµ�, �ش� �̵� ��ǥ�� ����
    for( int i = 0; i < MAX_LIGHTS; i++ )
        g_LightControl[i].SetRadius( fObjectRadius );

	//���̴��� �����͸� �޸𸮿� �������� �ʾ� ����Ʈ�� �޸� ��뷮�� 50% ����
    DWORD dwShaderFlags = D3DXFX_NOT_CLONEABLE;


	//�����Ϸ��� �̸� �˷��ִ� preshading �ɼ� �߰�
	//�׷��� �츮�� init���� g_bEnablePreshader�� true�� ������ ��ġ�� ����
    if( !g_bEnablePreshader )
        dwShaderFlags |= D3DXSHADER_NO_PRESHADER;

	//����Ʈ ������ ���ϸ��� ã��, �޾Ƽ� str�� ����(����ڵ� ����)
    WCHAR str[MAX_PATH];
    V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"BasicHLSL.fx" ) );

	//str�� �̿��ؼ� ���� shader�� g_pEffect�� ����
    V_RETURN( D3DXCreateEffectFromFile( pd3dDevice, str, NULL, NULL, dwShaderFlags, NULL, &g_pEffect, NULL ) );

    // �ؽ��� ���ϸ��� ã��, �޾Ƽ� str�� ����(����ڵ� ����)
    V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"tiny\\tiny_skin.dds" ) );

	//�ؽ��� �ҷ��ͼ� g_pMeshTexture�� ����
    V_RETURN( D3DXCreateTextureFromFileEx( pd3dDevice, str, D3DX_DEFAULT, D3DX_DEFAULT,
                                           D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED,
                                           D3DX_DEFAULT, D3DX_DEFAULT, 0,
                                           NULL, NULL, &g_pMeshTexture ) );

//�������� �ڵ� ���� ����
//�������� �����Ӹ��� �������ִ� ��
// 	//shader material�� ���� ��ġ ���� ��(����)
//     D3DXCOLOR colorMtrlDiffuse( 1.0f, 1.0f, 1.0f, 1.0f );
//     D3DXCOLOR colorMtrlAmbient( 0.35f, 0.35f, 0.35f, 0 );
// 
// 	//shader�� �غ�� ������ ���ε� ����
// 	//material ���� ��ġ, texture ��
//     V_RETURN( g_pEffect->SetValue( "g_MaterialAmbientColor", &colorMtrlAmbient, sizeof( D3DXCOLOR ) ) );
//     V_RETURN( g_pEffect->SetValue( "g_MaterialDiffuseColor", &colorMtrlDiffuse, sizeof( D3DXCOLOR ) ) );

    V_RETURN( g_pEffect->SetTexture( "g_MeshTexture", g_pMeshTexture ) );


    //ī�޶� ��ġ�� �����ϴ� �ڵ�
    D3DXVECTOR3 vecEye( 0.0f, 0.0f, -15.0f );
    D3DXVECTOR3 vecAt ( 0.0f, 0.0f, -0.0f );
    g_Camera.SetViewParams( &vecEye, &vecAt );

	//ī�޶� �̵��� �� �ִ� ���(����, ����) ����
    g_Camera.SetRadius( fObjectRadius * 3.0f, fObjectRadius * 0.5f, fObjectRadius * 10.0f );

    return S_OK;
}
//--------------------------------------------------------------------------------------
//�޽� �ҷ����� �Լ�
//--------------------------------------------------------------------------------------
HRESULT LoadMesh( IDirect3DDevice9* pd3dDevice, WCHAR* strFileName, ID3DXMesh** ppMesh )
{
    ID3DXMesh* pMesh = NULL;
    WCHAR str[MAX_PATH];
    HRESULT hr;

    V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, strFileName ) );
    V_RETURN( D3DXLoadMeshFromX( str, D3DXMESH_MANAGED, pd3dDevice, NULL, NULL, NULL, NULL, &pMesh ) );

    DWORD* rgdwAdjacency = NULL;

    // mesh�� �븻���� �����ϴ� �ڵ�
    if( !( pMesh->GetFVF() & D3DFVF_NORMAL ) )
    {
        ID3DXMesh* pTempMesh;
        V( pMesh->CloneMeshFVF( pMesh->GetOptions(),
                                pMesh->GetFVF() | D3DFVF_NORMAL,
                                pd3dDevice, &pTempMesh ) );
        V( D3DXComputeNormals( pTempMesh, NULL ) );

        SAFE_RELEASE( pMesh );
        pMesh = pTempMesh;
    }

	//���� ����� �����ϰ��� ���� ������ ���
	//�� mesh(�ﰢ��) ���� ���̺��� ������ ����
	//�ش� ������ pMesh�� ������ ����
	//�� mesh ������ ������ �� �ִ� ���� �ִ� ������ 3�� �̳����� Ȱ���� ȿ�������� vertex �
    rgdwAdjacency = new DWORD[pMesh->GetNumFaces() * 3];
    if( rgdwAdjacency == NULL )
        return E_OUTOFMEMORY;
    V( pMesh->GenerateAdjacency( 1e-6f, rgdwAdjacency ) );
	//���ؽ� ĳ���� Ȱ���ϴ� ��
    V( pMesh->OptimizeInplace( D3DXMESHOPT_VERTEXCACHE, rgdwAdjacency, NULL, NULL, NULL ) );
    delete []rgdwAdjacency;


	//callback out ������ ���� �� ����
    *ppMesh = pMesh;

    return S_OK;
}


//--------------------------------------------------------------------------------------
// ȭ�鿡 ��ȭ�� �߻��� �� ���ҽ� �籸�����ִ� �Լ�
// onlost�� �߻��ϸ� �� �Լ����� �� ���� �籸��
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnResetDevice( IDirect3DDevice9* pd3dDevice,
                                const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;
	
	// shader �籸��
    if( g_pEffect )
        V_RETURN( g_pEffect->OnResetDevice() );

	//���� �籸��
    for( int i = 0; i < MAX_LIGHTS; i++ )
        g_LightControl[i].OnD3D9ResetDevice( pBackBufferSurfaceDesc );
	
	//swap ��󿡼� �������� �޸� copy�� ���� ������ ȸ��
    float fAspectRatio = pBackBufferSurfaceDesc->Width / ( FLOAT )pBackBufferSurfaceDesc->Height;
    g_Camera.SetProjParams( D3DX_PI / 4, fAspectRatio, 2.0f, 4000.0f );
    g_Camera.SetWindow( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );
    g_Camera.SetButtonMasks( MOUSE_LEFT_BUTTON, MOUSE_WHEEL, MOUSE_MIDDLE_BUTTON );


    return S_OK;
}


//--------------------------------------------------------------------------------------
//������Ʈ ��� �Լ�
//������ �� �Ź� ȣ���� update ����
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
    //ī�޶� ��ġ ���� ���
    g_Camera.FrameMove( fElapsedTime );
}


//--------------------------------------------------------------------------------------
//���� render�� �߻��ϴ� �Լ�
//fTime, fElapsedTime�� ������ �ݹ��� �˾Ƽ� ���� ���� ����
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{

	//���� �ʿ� ����
    HRESULT hr;
    D3DXMATRIXA16 mWorldViewProjection;
    D3DXVECTOR3 vLightDir[MAX_LIGHTS];
    D3DXCOLOR vLightDiffuse[MAX_LIGHTS];
    UINT iPass, cPasses;
    D3DXMATRIXA16 mWorld;
    D3DXMATRIXA16 mView;
    D3DXMATRIXA16 mProj;

    //����̽� �ʱ�ȭ
    V( pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DXCOLOR( 0.0f, 0.25f, 0.25f, 0.55f ), 1.0f,
                          0 ) );

    //�� �׸��� ����
    if( SUCCEEDED( pd3dDevice->BeginScene() ) )
    {
        //ī�޶��� ���� ��Ʈ���� ���� �޾Ƽ� ���� ��Ʈ������ ���� ��� ��
        mWorld = g_mCenterWorld * (*g_Camera.GetWorldMatrix());
		//�������� ��Ʈ���� ����
        mProj = *g_Camera.GetProjMatrix();
		//�� ��Ʈ���� ����
        mView = *g_Camera.GetViewMatrix();

		//���� ��Ʈ������ *�� * �������� ��Ʈ���� ����
        mWorldViewProjection = mWorld * mView * mProj;

        //����Ʈ ����
        for( int i = 0; i < g_nNumActiveLights; i++ )
        {
            vLightDir[i] = g_LightControl[i].GetLightDirection();
            vLightDiffuse[i] = g_fLightScale * D3DXCOLOR( 1, 1, 1, 1 );
        }


		//shader�� ���� ���� ���� binding
        V( g_pEffect->SetValue( "g_LightDir", vLightDir, sizeof( D3DXVECTOR3 ) * MAX_LIGHTS ) );
        V( g_pEffect->SetValue( "g_LightDiffuse", vLightDiffuse, sizeof( D3DXVECTOR4 ) * MAX_LIGHTS ) );


		//shader�� ��Ʈ���� ���� ���� binding
        V( g_pEffect->SetMatrix( "g_mWorldViewProjection", &mWorldViewProjection ) );
        V( g_pEffect->SetMatrix( "g_mWorld", &mWorld ) );

		//shader�� ���� �Ӽ� �� ���� ���� binding
        D3DXCOLOR vWhite = D3DXCOLOR( 1, 1, 1, 1 );
        V( g_pEffect->SetValue( "g_MaterialDiffuseColor", &vWhite, sizeof( D3DXCOLOR ) ) );
        
		//shader�� time �� ����
		V( g_pEffect->SetFloat( "g_fTime", ( float )fTime ) );

		//shader�� Ȱ�� ���� ��
        V( g_pEffect->SetInt( "g_nNumLights", g_nNumActiveLights ) );

			
        switch( g_nNumActiveLights )
        {
            case 1:
                V( g_pEffect->SetTechnique( "RenderSceneWithTexture1Light" ) ); break;
            case 2:
                V( g_pEffect->SetTechnique( "RenderSceneWithTexture2Light" ) ); break;
            case 3:
                V( g_pEffect->SetTechnique( "RenderSceneWithTexture3Light" ) ); break;
        }


        //shader ����
        V( g_pEffect->Begin( &cPasses, 0 ) );
		
		//pass�� �׸��� ������ �ǹ�
		//�׸��� ���� ��ŭ pass ��ȯ
        for( iPass = 0; iPass < cPasses; iPass++ )
        {
            V( g_pEffect->BeginPass( iPass ) );

            // Render the mesh with the applied technique
            V( g_pMesh->DrawSubset( 0 ) );

            V( g_pEffect->EndPass() );
        }
        V( g_pEffect->End() );

		//�� �׸��� ����
        V( pd3dDevice->EndScene() );
    }
}




//--------------------------------------------------------------------------------------
//�޽��� �ݹ� �Լ�
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext )
{
	//���콺 ��Ʈ���� ȭ�鿡 �ݿ��Ǵ� �κ�
	//�� ��ü�� HandleMessages �Լ��� ������ �����Ǿ� �ִ� ����
	//�ݴ� �׼ǿ� ���� ���� ���� ��ü�� ���� ���� ex) g_LightControl�� HandleMessages�� R��ư�� ���ؼ��� ����


	//3�� �� ���õ� ���� ���� �Է� �� �޾Ƽ� ó���ϴ� �κ�
    g_LightControl[g_nActiveLight].HandleMessages( hWnd, uMsg, wParam, lParam );

	//L��ư�� Ȱ���� �� ȸ�� ó�� ����
    g_Camera.HandleMessages( hWnd, uMsg, wParam, lParam );

    return 0;
}



//--------------------------------------------------------------------------------------
// d3d ������ �ʿ��� �ڿ��� �߰��� ������� �� ������ �� ���, reset�ϴ� �Լ�
//--------------------------------------------------------------------------------------
void CALLBACK OnLostDevice( void* pUserContext )
{
	
	//ex) ��ü ȭ�� ���ٰ� ���� ȭ������ ���ƿ��� ���
    CDXUTDirectionWidget::StaticOnD3D9LostDevice();

    if( g_pEffect )
        g_pEffect->OnLostDevice();

}


//--------------------------------------------------------------------------------------
// �ڿ� ���� �Լ� 
//--------------------------------------------------------------------------------------
void CALLBACK OnDestroyDevice( void* pUserContext )
{
    //d3d���� device, buffer ���� wrapping�ϰ� �ִ� class ����
	CDXUTDirectionWidget::StaticOnD3D9DestroyDevice();
	
	//���̴� ��ü
    SAFE_RELEASE( g_pEffect );
    SAFE_RELEASE( g_pMesh );
    SAFE_RELEASE( g_pMeshTexture );
}



