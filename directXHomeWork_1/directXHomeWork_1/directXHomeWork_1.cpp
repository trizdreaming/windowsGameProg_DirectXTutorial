// directXHomeWork_1.cpp : 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "directXHomeWork_1.h"

#include <d3dx9.h>
#include <strsafe.h>
#include <timeapi.h>

LPDIRECT3D9 g_pD3D = NULL;
LPDIRECT3DDEVICE9 g_pD3DDevice = NULL;
LPDIRECT3DVERTEXBUFFER9 g_pVertexBuffer0 = NULL;
LPDIRECT3DVERTEXBUFFER9 g_pVertexBuffer1 = NULL;

LPDIRECT3DTEXTURE9 g_pTexture0 = NULL;
LPDIRECT3DTEXTURE9 g_pTexture1 = NULL;

LPD3DXMESH g_pMesh0 = NULL;
D3DMATERIAL9* g_pMeshMaterials0 = NULL;
LPDIRECT3DTEXTURE9* g_pMeshTextures0 = NULL;
DWORD g_dwNumMaterials = 0L;


UINT g_tick = 0;

struct CUSTOMVERTEX
{
	D3DXVECTOR3 position;
	D3DXVECTOR3 normal;
	D3DCOLOR color;

	FLOAT tu, tv;
};

//#define D3DFVF_CUSTOMVERTEX ( D3DFVF_XYZ|D3DFVF_NORMAL )
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE|D3DFVF_TEX1)
//#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)
//#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1)

HRESULT InitD3D(HWND hWnd);
HRESULT InitGeometry();
VOID SetupMatrices();
VOID SetupLights();
VOID Render();
VOID Cleanup();


LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		Cleanup();
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

//main 함수
INT WINAPI wWinMain( HINSTANCE hInst, HINSTANCE, LPWSTR, INT )
{
	hInst;

	WNDCLASSEX wc =
	{
		sizeof( WNDCLASSEX ), CS_CLASSDC, MsgProc, 0L, 0L,
		GetModuleHandle(NULL), NULL, LoadCursor(NULL, IDC_ARROW), NULL, NULL,
		L"D3D Tutorial", NULL
	};
	RegisterClassEx( &wc );

	HWND hWnd = CreateWindow(L"D3D Tutorial", L"D3D HomeWork", WS_OVERLAPPEDWINDOW, 100, 100, 600, 600, NULL, NULL, wc.hInstance, NULL);

	if (SUCCEEDED(InitD3D(hWnd)))
	{
		if (SUCCEEDED(InitGeometry()))
		{
			ShowWindow(hWnd, SW_SHOWDEFAULT);
			UpdateWindow(hWnd);

			MSG msg;
			ZeroMemory(&msg, sizeof(msg));

			while (msg.message != WM_QUIT)
			{
				if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				else
				{
					Render();
				}
			}
		}
	}

	UnregisterClass(L"D3D Tutorial", wc.hInstance);
	return 0;
}

HRESULT InitD3D(HWND hWnd)
{
	if (NULL == (g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)))
	{
		return E_FAIL;
	}

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

	if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &g_pD3DDevice)))
	{
		return E_FAIL;
	}

	g_pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	g_pD3DDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	//g_pD3DDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

	return S_OK;
}

HRESULT InitGeometry()
{
	//mesh
	LPD3DXBUFFER pD3DXMtrlBuffer0;

	if (FAILED(D3DXLoadMeshFromX(L"Tiger.x", D3DXMESH_SYSTEMMEM, g_pD3DDevice, NULL, &pD3DXMtrlBuffer0, NULL, &g_dwNumMaterials, &g_pMesh0)))
	{
		MessageBox(NULL, L"Could not find tiger.x", L"Meshes.exe", MB_OK);
		return E_FAIL;
	}

	//1번
	D3DXMATERIAL* d3dxMarteials = (D3DXMATERIAL*) pD3DXMtrlBuffer0->GetBufferPointer();
	g_pMeshMaterials0 = new D3DMATERIAL9[g_dwNumMaterials];
	if (NULL == g_pMeshMaterials0)
	{
		return E_OUTOFMEMORY;
	}
	g_pMeshTextures0 = new LPDIRECT3DTEXTURE9[g_dwNumMaterials];
	if (NULL == g_pMeshTextures0)
	{
		return E_OUTOFMEMORY;
	}

	for (DWORD i = 0; i < g_dwNumMaterials; ++i)
	{
		g_pMeshMaterials0[i] = d3dxMarteials[i].MatD3D;

		g_pMeshMaterials0[i].Ambient = g_pMeshMaterials0[i].Diffuse;

		g_pMeshTextures0[i] = NULL;
		if ((NULL != d3dxMarteials[i].pTextureFilename) && lstrlenA(d3dxMarteials[i].pTextureFilename)>0)
		{
			if (FAILED(D3DXCreateTextureFromFileA(g_pD3DDevice, d3dxMarteials[i].pTextureFilename, &g_pMeshTextures0[i])))
			{
				const CHAR* strPrefix = "..\\";
				CHAR strTexture[MAX_PATH];
				strcpy_s(strTexture, MAX_PATH, strPrefix);
				strcat_s(strTexture, MAX_PATH, d3dxMarteials[i].pTextureFilename);

				if (FAILED(D3DXCreateTextureFromFileA(g_pD3DDevice, strTexture, &g_pMeshTextures0[i])))
				{
					MessageBox(NULL, L"Could not find texture map", L"Meshes.exe", MB_OK);
				}
			}
		}
	}
	pD3DXMtrlBuffer0->Release();

	//silinder
	if (FAILED(D3DXCreateTextureFromFile(g_pD3DDevice, L"gom.bmp", &g_pTexture0)))
	{
		MessageBox(NULL, L"Could not find gom.bmp", L"*.exe", MB_OK);
		return E_FAIL;
	}

	if (FAILED(D3DXCreateTextureFromFile(g_pD3DDevice, L"sun.bmp", &g_pTexture1)))
	{
		MessageBox(NULL, L"Could not find sun.bmp", L"*.exe", MB_OK);
		return E_FAIL;
	}

	//buffer0
	if (FAILED(g_pD3DDevice->CreateVertexBuffer(50*2*sizeof(CUSTOMVERTEX), 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &g_pVertexBuffer0, NULL)))
	{
		return E_FAIL;
	}

	CUSTOMVERTEX* pVertieces;
	if (FAILED(g_pVertexBuffer0->Lock(0, 0, (void**)&pVertieces, 0)))
	{
		return E_FAIL;
	}
	for (DWORD i = 0; i < 50; ++i)
	{
		FLOAT theta = (2 * D3DX_PI*i) / (50 - 1);
		pVertieces[2 * i + 0].position = D3DXVECTOR3(sinf(theta)-0.7f, 1.5f, cosf(theta));
		pVertieces[2 * i + 0].normal = D3DXVECTOR3(sinf(theta), 0.f, cosf(theta));
		pVertieces[2 * i + 0].color = 0xffffffff;
		
		pVertieces[2 * i + 0].tu = ((FLOAT) i) / (50 - 1);
		pVertieces[2 * i + 0].tv = 1.f;
		
		pVertieces[2 * i + 1].position = D3DXVECTOR3(sinf(theta)-0.7f, 3.5f, cosf(theta));
		pVertieces[2 * i + 1].normal = D3DXVECTOR3(sinf(theta), 0.f, cosf(theta));
		pVertieces[2 * i + 1].color = 0xffffffff;

		pVertieces[2 * i + 1].tu = ((FLOAT) i) / (50 - 1);
		pVertieces[2 * i + 1].tv = 0.f;
	}
	g_pVertexBuffer0->Unlock();

	//buffer1
	if (FAILED(g_pD3DDevice->CreateVertexBuffer(50 * 2 * sizeof(CUSTOMVERTEX), 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &g_pVertexBuffer1, NULL)))
	{
		return E_FAIL;
	}

	CUSTOMVERTEX* pVertieces1;
	if (FAILED(g_pVertexBuffer1->Lock(0, 0, (void**) &pVertieces1, 0)))
	{
		return E_FAIL;
	}
	for (DWORD i = 0; i < 50; ++i)
	{
		FLOAT theta = (2 * D3DX_PI*i) / (50 - 1);
		pVertieces1[2 * i + 0].position = D3DXVECTOR3(sinf(theta)+.7f, 1.5f, cosf(theta));
		pVertieces1[2 * i + 0].normal = D3DXVECTOR3(sinf(theta), 0.f, cosf(theta));
		pVertieces1[2 * i + 0].color = 0xffffffff;

		pVertieces1[2 * i + 0].tu = ((FLOAT) i) / (50 - 1);
		pVertieces1[2 * i + 0].tv = 1.f;

		pVertieces1[2 * i + 1].position = D3DXVECTOR3(sinf(theta)+.7f, 3.5f, cosf(theta));
		pVertieces1[2 * i + 1].normal = D3DXVECTOR3(sinf(theta), 0.f, cosf(theta));
		pVertieces1[2 * i + 1].color = 0xffffffff;

		pVertieces1[2 * i + 1].tu = ((FLOAT) i) / (50 - 1);
		pVertieces1[2 * i + 1].tv = 0.f;
	}
	g_pVertexBuffer1->Unlock();

	return S_OK;
}

VOID SetupMatrices()
{
	D3DXMATRIXA16 matWorld;
	D3DXMatrixIdentity(&matWorld);
	g_pD3DDevice->SetTransform(D3DTS_WORLD, &matWorld);

	D3DXVECTOR3 vEyePt(0.f, 3.f, -5.f);
	D3DXVECTOR3 vLookatPt(0.f, 0.f, 0.f);
	D3DXVECTOR3 vUpVec(0.f, 1.f, 0.f);
	D3DXMATRIXA16 matView;
	D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);
	g_pD3DDevice->SetTransform(D3DTS_VIEW, &matView);

	D3DXMATRIXA16 matProj;
	D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI/2, 1.0f, 1.0f, 100.0f);
	g_pD3DDevice->SetTransform(D3DTS_PROJECTION, &matProj);
}

VOID SetupLights()
{
	D3DMATERIAL9 mtrl;
	ZeroMemory(&mtrl, sizeof(D3DMATERIAL9));
	mtrl.Diffuse.r = mtrl.Ambient.r = 1.0f;
	mtrl.Diffuse.g = mtrl.Ambient.g = 1.0f;
	mtrl.Diffuse.b = mtrl.Ambient.b = 1.0f;
	mtrl.Diffuse.a = mtrl.Ambient.a = 1.0f;
	g_pD3DDevice->SetMaterial(&mtrl);

	D3DXVECTOR3 vecDir0;
	D3DLIGHT9 light0;
	ZeroMemory(&light0, sizeof(D3DLIGHT9));
	light0.Type = D3DLIGHT_DIRECTIONAL;
	light0.Diffuse.r = 1.f;
	light0.Diffuse.g = 1.f;
	light0.Diffuse.b = 1.f;
	vecDir0 = D3DXVECTOR3(-1.f, 1.f, 0.f );
	D3DXVec3Normalize((D3DXVECTOR3*) &light0.Direction, &vecDir0);
	light0.Range = 1000.f;


	D3DXVECTOR3 vecDir1;
	D3DLIGHT9 light1;
	ZeroMemory(&light1, sizeof(D3DLIGHT9));
	light1.Type = D3DLIGHT_DIRECTIONAL;
	light1.Diffuse.r = 1.f;
	light1.Diffuse.g = 1.f;
	light1.Diffuse.b = 1.f;
	vecDir1 = D3DXVECTOR3(1.f, 1.f, 0.f);
	D3DXVec3Normalize((D3DXVECTOR3*) &light1.Direction, &vecDir1);
	light1.Range = 1000.f;

	g_pD3DDevice->SetLight(0, &light0);
	g_pD3DDevice->SetLight(1, &light1);

	g_pD3DDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
	g_pD3DDevice->SetRenderState(D3DRS_AMBIENT, 0x00808080);

	if (5<(g_tick%31))
	{
		g_pD3DDevice->LightEnable(0, TRUE);
		g_pD3DDevice->LightEnable(1, FALSE);
	}
	else
	{
		g_pD3DDevice->LightEnable(0, FALSE);
		g_pD3DDevice->LightEnable(1, TRUE);
	}
}

VOID Render()
{
	g_pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 255), 1.0f, 0);

	if (SUCCEEDED(g_pD3DDevice->BeginScene()))
	{
		SetupLights();

		SetupMatrices();

		if (15 < (g_tick % 31))
		{
			g_pD3DDevice->SetTexture(0, g_pTexture0);
		}
		else
		{
			g_pD3DDevice->SetTexture(0, g_pTexture1);
		}
		g_pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		g_pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		g_pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		g_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
		

		g_pD3DDevice->SetStreamSource(0, g_pVertexBuffer0, 0, sizeof(CUSTOMVERTEX));
		g_pD3DDevice->SetFVF(D3DFVF_CUSTOMVERTEX);
		g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2 * 50 - 2);

		g_pD3DDevice->SetStreamSource(0, g_pVertexBuffer1, 0, sizeof(CUSTOMVERTEX));
		g_pD3DDevice->SetFVF(D3DFVF_CUSTOMVERTEX);
		g_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2 * 50 - 2);



		//tiger 1
		D3DXMATRIXA16 matFirstTiger, matTrans, matRotate;
		//D3DXMatrixIdentity(&matFirstTiger);

		g_pD3DDevice->GetTransform(D3DTS_WORLD, &matFirstTiger);
		D3DXMatrixTranslation(&matTrans, -3.f, -1.5f, -1.6f);		
		D3DXMatrixRotationY(&matRotate, timeGetTime() / 1000.0f);
		D3DXMatrixMultiply(&matFirstTiger, &matRotate, &matTrans);
		g_pD3DDevice->SetTransform(D3DTS_WORLD, &matFirstTiger);

		
		for (DWORD i = 0; i < g_dwNumMaterials; ++i)
		{
			g_pD3DDevice->SetMaterial(&g_pMeshMaterials0[i]);
			g_pD3DDevice->SetTexture(0, g_pMeshTextures0[i]);

			g_pMesh0->DrawSubset(i);
		}


		//tiger 2
		D3DXMATRIXA16 matSecondTiger;
		D3DXMatrixIdentity(&matSecondTiger);
		D3DXMatrixIdentity(&matTrans);
		D3DXMatrixIdentity(&matRotate);

		g_pD3DDevice->GetTransform(D3DTS_WORLD, &matSecondTiger);
		D3DXMatrixTranslation(&matTrans, 3.f, -1.5f, -1.6f);
		D3DXMatrixRotationY(&matRotate, -(timeGetTime() / 1000.0f));
		D3DXMatrixMultiply(&matSecondTiger, &matRotate, &matTrans);
		g_pD3DDevice->SetTransform(D3DTS_WORLD, &matSecondTiger);

		for (DWORD i = 0; i < g_dwNumMaterials; ++i)
		{
			g_pD3DDevice->SetMaterial(&g_pMeshMaterials0[i]);
			g_pD3DDevice->SetTexture(0, g_pMeshTextures0[i]);

			g_pMesh0->DrawSubset(i);
		}

		g_pD3DDevice->EndScene();
	}

	g_pD3DDevice->Present(NULL, NULL, NULL, NULL);
	g_tick++;
}

VOID Cleanup()
{
	if (NULL != g_pMeshMaterials0)
	{
		delete [] g_pMeshMaterials0;
	}
	if (g_pMeshTextures0)
	{
		for (DWORD i = 0; i < g_dwNumMaterials; ++i)
		{
			if (g_pMeshTextures0[i])
			{
				g_pMeshTextures0[i]->Release();
			}
		}
		delete [] g_pMeshTextures0;
	}
	if (NULL != g_pMesh0)
	{
		g_pMesh0->Release();
	}


	if (NULL != g_pTexture0)
	{
		g_pTexture0->Release();
	}
	if (NULL != g_pTexture1)
	{
		g_pTexture1->Release();
	}
	if (NULL != g_pVertexBuffer0)
	{
		g_pVertexBuffer0->Release();
	}
	if (NULL != g_pVertexBuffer1)
	{
		g_pVertexBuffer1->Release();
	}

	if (NULL != g_pD3DDevice)
	{
		g_pD3DDevice->Release();
	}

	if (NULL != g_pD3D)
	{
		g_pD3D->Release();
	}
}