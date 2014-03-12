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
// 글로벌 변수
//--------------------------------------------------------------------------------------
CModelViewerCamera          g_Camera;               // 카메라 등 (눈으로 보는 것)
ID3DXEffect*                g_pEffect = NULL;       // 
ID3DXMesh*                  g_pMesh = NULL;         // Mesh object
IDirect3DTexture9*          g_pMeshTexture = NULL;  // Mesh texture

//최적화 옵션
bool                        g_bEnablePreshader;     // if TRUE, then D3DXSHADER_NO_PRESHADER is used when compiling the shader

D3DXMATRIXA16               g_mCenterWorld; //월드 매트릭스

//최대 조명 개수 3개
#define MAX_LIGHTS 3
//조명 관련 변수가 Warpping 되어 있는 객체
//3d3 관련 객체(device) 등도 같이 포함되어 있음
//단, 여기서 바로 사용되는 것은 아님
CDXUTDirectionWidget g_LightControl[MAX_LIGHTS];

//조명 밝기
float                       g_fLightScale;
//활성 조명 개수
int                         g_nNumActiveLights;
//현재 활성화 된 조명
int                         g_nActiveLight;


//--------------------------------------------------------------------------------------
// Forward declarations 
// 컴파일러에게 함수 리스트 선언
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


// main 함수 임
INT WINAPI wWinMain( HINSTANCE, HINSTANCE, LPWSTR, int )
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    DXUTSetCallbackD3D9DeviceCreated( OnCreateDevice );
    DXUTSetCallbackD3D9DeviceReset( OnResetDevice );
    DXUTSetCallbackD3D9FrameRender( OnFrameRender );
	
	//focus를 상실했을 때. 즉 창모드/전체화면 모드 변경시 처리 과정
    DXUTSetCallbackD3D9DeviceLost( OnLostDevice );
    DXUTSetCallbackD3D9DeviceDestroyed( OnDestroyDevice );
    DXUTSetCallbackMsgProc( MsgProc );

	//update 역할, 렌더 직전에 변경사항 적용하는 부분
	DXUTSetCallbackFrameMove( OnFrameMove );

	//조명 초기화
    InitApp();

	//키보드 단축키를 받아 처리하도록 하는 플래그 설정 함수
	DXUTSetHotkeyHandling( true, true, true );

    DXUTCreateWindow( L"BasicHLSL" );
    DXUTCreateDevice( true, 640, 480 );

	// main 무한 루프
	// 무한 루프 중 세팅되는 값들을 적용해 화면에 표시함
    DXUTMainLoop();


    return DXUTGetExitCode();
}


//--------------------------------------------------------------------------------------
// Initialize the app 
//--------------------------------------------------------------------------------------
void InitApp()
{
    //컴파일러에게 쉐이더 사용 여부를 미리 알림
	//더 효율적인 컴파일러 움직임을 위해
	g_bEnablePreshader = true;

	//각 조명의 초기 값 세팅
    for( int i = 0; i < MAX_LIGHTS; i++ )
        g_LightControl[i].SetLightDirection( D3DXVECTOR3( sinf( D3DX_PI * 2 * i / MAX_LIGHTS - D3DX_PI / 6 ),
                                                          0, -cosf( D3DX_PI * 2 * i / MAX_LIGHTS - D3DX_PI / 6 ) ) );

	//초기 활성화 된 조명 idx 번호
    g_nActiveLight = 0;

	//초기 활성화할 조명 개수
    g_nNumActiveLights = 1;
    
	//조명 밝기 값
	g_fLightScale = 1.0f;

}


//--------------------------------------------------------------------------------------
// 디바이스 생성 함수
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
                                 void* pUserContext )
{
    HRESULT hr;

    //mesh 불러와서 g_pMesh에 저장
    V_RETURN( LoadMesh( pd3dDevice, L"tiny\\tiny.x", &g_pMesh ) );

    D3DXVECTOR3* pData;
    D3DXVECTOR3 vCenter;
    FLOAT fObjectRadius;

	//메쉬를 쓸 buffer에 대한 lock
	//lock은 버텍스 버퍼의 위치 값을 불러줌(어디에 있는지 상관 없이 리턴)
	//첫 번째 인자가 버퍼 옵션(3= readonly)
    V( g_pMesh->LockVertexBuffer( 3, ( LPVOID* )&pData ) );
	//메쉬 경계값 산출 = 모든 vertex 순회
	//모든 vertex순회 후 평균 계산= 중점, 가장 먼거리의 vertex가 반지름
    V( D3DXComputeBoundingSphere( pData, g_pMesh->GetNumVertices(),
                                 D3DXGetFVFVertexSize( g_pMesh->GetFVF() ), &vCenter, &fObjectRadius ) );
	//buffer unlock
    V( g_pMesh->UnlockVertexBuffer() );


	//월드 좌표 초기 세팅
	//mesh에서 받은 중앙값을 기준으로 세팅
    D3DXMatrixTranslation( &g_mCenterWorld, -vCenter.x, -vCenter.y, -vCenter.z );
    D3DXMATRIXA16 m;
    D3DXMatrixRotationY( &m, D3DX_PI );
    g_mCenterWorld *= m;
    D3DXMatrixRotationX( &m, D3DX_PI / 2.0f );
    g_mCenterWorld *= m;


	//warping된 device객체 생성
    V_RETURN( CDXUTDirectionWidget::StaticOnD3D9CreateDevice( pd3dDevice ) );

	//조명 이동 형태가 구를 이루고 있는데, 해당 이동 좌표를 결정
    for( int i = 0; i < MAX_LIGHTS; i++ )
        g_LightControl[i].SetRadius( fObjectRadius );

	//셰이더의 데이터를 메모리에 보존하지 않아 이펙트의 메모리 사용량을 50% 줄임
    DWORD dwShaderFlags = D3DXFX_NOT_CLONEABLE;


	//컴파일러에 미리 알려주는 preshading 옵션 추가
	//그런데 우리는 init에서 g_bEnablePreshader를 true로 했으니 거치지 않음
    if( !g_bEnablePreshader )
        dwShaderFlags |= D3DXSHADER_NO_PRESHADER;

	//이펙트 파일의 파일명을 찾고, 받아서 str에 저장(방어코드 개념)
    WCHAR str[MAX_PATH];
    V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"BasicHLSL.fx" ) );

	//str을 이용해서 실제 shader를 g_pEffect에 넣음
    V_RETURN( D3DXCreateEffectFromFile( pd3dDevice, str, NULL, NULL, dwShaderFlags, NULL, &g_pEffect, NULL ) );

    // 텍스쳐 파일명을 찾고, 받아서 str에 저장(방어코드 개념)
    V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"tiny\\tiny_skin.dds" ) );

	//텍스쳐 불러와서 g_pMeshTexture에 저장
    V_RETURN( D3DXCreateTextureFromFileEx( pd3dDevice, str, D3DX_DEFAULT, D3DX_DEFAULT,
                                           D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED,
                                           D3DX_DEFAULT, D3DX_DEFAULT, 0,
                                           NULL, NULL, &g_pMeshTexture ) );

//쓸데없는 코드 같아 보임
//렌더에서 프레임마다 설정해주는 중
// 	//shader material에 조명 수치 적용 값(예정)
//     D3DXCOLOR colorMtrlDiffuse( 1.0f, 1.0f, 1.0f, 1.0f );
//     D3DXCOLOR colorMtrlAmbient( 0.35f, 0.35f, 0.35f, 0 );
// 
// 	//shader와 준비된 설정값 바인딩 과정
// 	//material 조명 수치, texture 등
//     V_RETURN( g_pEffect->SetValue( "g_MaterialAmbientColor", &colorMtrlAmbient, sizeof( D3DXCOLOR ) ) );
//     V_RETURN( g_pEffect->SetValue( "g_MaterialDiffuseColor", &colorMtrlDiffuse, sizeof( D3DXCOLOR ) ) );

    V_RETURN( g_pEffect->SetTexture( "g_MeshTexture", g_pMeshTexture ) );


    //카메라 위치를 설정하는 코드
    D3DXVECTOR3 vecEye( 0.0f, 0.0f, -15.0f );
    D3DXVECTOR3 vecAt ( 0.0f, 0.0f, -0.0f );
    g_Camera.SetViewParams( &vecEye, &vecAt );

	//카메라가 이동할 수 있는 경로(구형, 경계면) 설정
    g_Camera.SetRadius( fObjectRadius * 3.0f, fObjectRadius * 0.5f, fObjectRadius * 10.0f );

    return S_OK;
}
//--------------------------------------------------------------------------------------
//메쉬 불러오는 함수
//--------------------------------------------------------------------------------------
HRESULT LoadMesh( IDirect3DDevice9* pd3dDevice, WCHAR* strFileName, ID3DXMesh** ppMesh )
{
    ID3DXMesh* pMesh = NULL;
    WCHAR str[MAX_PATH];
    HRESULT hr;

    V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, strFileName ) );
    V_RETURN( D3DXLoadMeshFromX( str, D3DXMESH_MANAGED, pd3dDevice, NULL, NULL, NULL, NULL, &pMesh ) );

    DWORD* rgdwAdjacency = NULL;

    // mesh에 노말벡터 생성하는 코드
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

	//성능 향상을 도모하고자 인접 정보를 기록
	//각 mesh(삼각형) 정보 테이블을 가지는 형태
	//해당 정보는 pMesh가 가지고 있음
	//각 mesh 정보는 인접할 수 있는 점의 최대 개수가 3개 이내임을 활용해 효율적으로 vertex 운영
    rgdwAdjacency = new DWORD[pMesh->GetNumFaces() * 3];
    if( rgdwAdjacency == NULL )
        return E_OUTOFMEMORY;
    V( pMesh->GenerateAdjacency( 1e-6f, rgdwAdjacency ) );
	//버텍스 캐쉬를 활용하는 것
    V( pMesh->OptimizeInplace( D3DXMESHOPT_VERTEXCACHE, rgdwAdjacency, NULL, NULL, NULL ) );
    delete []rgdwAdjacency;


	//callback out 변수에 최종 값 저장
    *ppMesh = pMesh;

    return S_OK;
}


//--------------------------------------------------------------------------------------
// 화면에 변화가 발생할 때 리소스 재구성해주는 함수
// onlost가 발생하면 이 함수에서 상세 내용 재구성
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnResetDevice( IDirect3DDevice9* pd3dDevice,
                                const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;
	
	// shader 재구성
    if( g_pEffect )
        V_RETURN( g_pEffect->OnResetDevice() );

	//조명 재구성
    for( int i = 0; i < MAX_LIGHTS; i++ )
        g_LightControl[i].OnD3D9ResetDevice( pBackBufferSurfaceDesc );
	
	//swap 대상에서 전면으로 메모리 copy를 통해 정보값 회복
    float fAspectRatio = pBackBufferSurfaceDesc->Width / ( FLOAT )pBackBufferSurfaceDesc->Height;
    g_Camera.SetProjParams( D3DX_PI / 4, fAspectRatio, 2.0f, 4000.0f );
    g_Camera.SetWindow( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );
    g_Camera.SetButtonMasks( MOUSE_LEFT_BUTTON, MOUSE_WHEEL, MOUSE_MIDDLE_BUTTON );


    return S_OK;
}


//--------------------------------------------------------------------------------------
//업데이트 기능 함수
//렌더링 전 매번 호출해 update 진행
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
    //카메라 위치 변경 계산
    g_Camera.FrameMove( fElapsedTime );
}


//--------------------------------------------------------------------------------------
//실제 render가 발생하는 함수
//fTime, fElapsedTime은 윈도우 콜백이 알아서 만들어서 인자 전달
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{

	//렌더 필요 변수
    HRESULT hr;
    D3DXMATRIXA16 mWorldViewProjection;
    D3DXVECTOR3 vLightDir[MAX_LIGHTS];
    D3DXCOLOR vLightDiffuse[MAX_LIGHTS];
    UINT iPass, cPasses;
    D3DXMATRIXA16 mWorld;
    D3DXMATRIXA16 mView;
    D3DXMATRIXA16 mProj;

    //디바이스 초기화
    V( pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DXCOLOR( 0.0f, 0.25f, 0.25f, 0.55f ), 1.0f,
                          0 ) );

    //씬 그리기 시작
    if( SUCCEEDED( pd3dDevice->BeginScene() ) )
    {
        //카메라의 현재 매트릭스 값을 받아서 월드 매트릭스에 곱한 결과 값
        mWorld = g_mCenterWorld * (*g_Camera.GetWorldMatrix());
		//프로젝션 매트릭스 설정
        mProj = *g_Camera.GetProjMatrix();
		//뷰 매트릭스 설정
        mView = *g_Camera.GetViewMatrix();

		//월드 매트릭스에 *뷰 * 프로젝션 매트릭스 적용
        mWorldViewProjection = mWorld * mView * mProj;

        //라이트 적용
        for( int i = 0; i < g_nNumActiveLights; i++ )
        {
            vLightDir[i] = g_LightControl[i].GetLightDirection();
            vLightDiffuse[i] = g_fLightScale * D3DXCOLOR( 1, 1, 1, 1 );
        }


		//shader에 조명 관련 정보 binding
        V( g_pEffect->SetValue( "g_LightDir", vLightDir, sizeof( D3DXVECTOR3 ) * MAX_LIGHTS ) );
        V( g_pEffect->SetValue( "g_LightDiffuse", vLightDiffuse, sizeof( D3DXVECTOR4 ) * MAX_LIGHTS ) );


		//shader에 매트릭스 관련 정보 binding
        V( g_pEffect->SetMatrix( "g_mWorldViewProjection", &mWorldViewProjection ) );
        V( g_pEffect->SetMatrix( "g_mWorld", &mWorld ) );

		//shader에 재질 속성 값 관련 정보 binding
        D3DXCOLOR vWhite = D3DXCOLOR( 1, 1, 1, 1 );
        V( g_pEffect->SetValue( "g_MaterialDiffuseColor", &vWhite, sizeof( D3DXCOLOR ) ) );
        
		//shader에 time 값 전달
		V( g_pEffect->SetFloat( "g_fTime", ( float )fTime ) );

		//shader에 활성 조명 값
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


        //shader 시작
        V( g_pEffect->Begin( &cPasses, 0 ) );
		
		//pass는 그리는 개수를 의미
		//그리는 개수 만큼 pass 순환
        for( iPass = 0; iPass < cPasses; iPass++ )
        {
            V( g_pEffect->BeginPass( iPass ) );

            // Render the mesh with the applied technique
            V( g_pMesh->DrawSubset( 0 ) );

            V( g_pEffect->EndPass() );
        }
        V( g_pEffect->End() );

		//씬 그리기 종료
        V( pd3dDevice->EndScene() );
    }
}




//--------------------------------------------------------------------------------------
//메시지 콜백 함수
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext )
{
	//마우스 컨트롤이 화면에 반영되는 부분
	//각 객체의 HandleMessages 함수가 별도로 구현되어 있는 형태
	//반대 액션에 대한 것은 구현 자체를 하지 않음 ex) g_LightControl의 HandleMessages는 R버튼에 대해서만 구현


	//3개 중 선택된 조명에 대한 입력 값 받아서 처리하는 부분
    g_LightControl[g_nActiveLight].HandleMessages( hWnd, uMsg, wParam, lParam );

	//L버튼을 활용한 모델 회전 처리 구현
    g_Camera.HandleMessages( hWnd, uMsg, wParam, lParam );

    return 0;
}



//--------------------------------------------------------------------------------------
// d3d 구동에 필요한 자원이 중간에 사라지는 등 문제가 된 경우, reset하는 함수
//--------------------------------------------------------------------------------------
void CALLBACK OnLostDevice( void* pUserContext )
{
	
	//ex) 전체 화면 갔다가 작은 화면으로 돌아오는 경우
    CDXUTDirectionWidget::StaticOnD3D9LostDevice();

    if( g_pEffect )
        g_pEffect->OnLostDevice();

}


//--------------------------------------------------------------------------------------
// 자원 해제 함수 
//--------------------------------------------------------------------------------------
void CALLBACK OnDestroyDevice( void* pUserContext )
{
    //d3d관련 device, buffer 등을 wrapping하고 있는 class 해제
	CDXUTDirectionWidget::StaticOnD3D9DestroyDevice();
	
	//셰이더 객체
    SAFE_RELEASE( g_pEffect );
    SAFE_RELEASE( g_pMesh );
    SAFE_RELEASE( g_pMeshTexture );
}



