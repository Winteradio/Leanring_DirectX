#include "DXEngine.h"

DXENGINE::DXENGINE(){}

DXENGINE::DXENGINE( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd )
{
	bool hr;

	hr = this->InitWindow( hInstance, nShowCmd, this->m_Width, this->m_Height, this->m_Fullscreen );
	if ( !hr )
	{
		LOG_ERROR(" Initalization Window - Failed \n ");
		this->Cleanup();
	}
	else
	{
		LOG_INFO(" Initalization Window - Successed \n ");
	}


	hr = this->InitD3D11();
	if ( !hr )
	{
		LOG_ERROR(" Initalization D3D11 - Failed \n ");
		this->Cleanup();
	}
	else
	{
		LOG_INFO(" Initalization D3D11 - Successed \n ");
	}

	hr = this->InitScene();
	if ( !hr )
	{
		LOG_ERROR(" Initalization Scene - Failed \n ");
		this->Cleanup();
	}
	else
	{
		LOG_INFO(" Initalization Scene - Successed \n ");
	}
}

DXENGINE::~DXENGINE()
{
	Cleanup();
}

bool DXENGINE::InitD3D11()
{
	HRESULT hr;

	// Describe our Buffer
	DXGI_MODE_DESC bufferDesc;

	ZeroMemory( &bufferDesc, sizeof( DXGI_MODE_DESC ) );

	bufferDesc.Width = this->m_Width;
	bufferDesc.Height = this->m_Height;
	bufferDesc.RefreshRate.Numerator = 60;
	bufferDesc.RefreshRate.Denominator = 1;
	bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;


	// Describe out SwapChain
	DXGI_SWAP_CHAIN_DESC swapChainDesc;

	ZeroMemory( &swapChainDesc, sizeof( DXGI_SWAP_CHAIN_DESC ) );

	swapChainDesc.BufferDesc = bufferDesc;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.OutputWindow = this->m_hWnd;
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;


	// Create our Device and SwapChain
	hr = D3D11CreateDeviceAndSwapChain( NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, NULL, NULL, D3D11_SDK_VERSION, &swapChainDesc, &(this->m_SwapChain), &(this->m_D3D11Device), NULL, &(this->m_D3D11DevCon) );
	if ( FAILED( hr ) )
	{
		LOG_ERROR(" Create Device and SwapChain - Failed \n ");
		return false;
	}


	// Create the BackBuffer
	ID3D11Texture2D* backBuffer;

	hr = this->m_SwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (void**)&backBuffer );
	if ( FAILED( hr ) )
	{
		LOG_ERROR(" SwapChain Get BackBuffer - Failed \n ");
		return false;
	}


	// Create Depth/Stencil Buffer Description
	D3D11_TEXTURE2D_DESC depthStencilDesc;

	ZeroMemory( &depthStencilDesc, sizeof( D3D11_TEXTURE2D_DESC ) );

	depthStencilDesc.Width = this->m_Width;
	depthStencilDesc.Height = this->m_Height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	this->m_D3D11Device->CreateTexture2D( &depthStencilDesc, NULL, &(this->m_DepthStencilBuffer) );
	this->m_D3D11Device->CreateDepthStencilView( this->m_DepthStencilBuffer, NULL, &(this->m_DepthStencilView) );


	// Create the Render Target
	hr = this->m_D3D11Device->CreateRenderTargetView( backBuffer, NULL, &(this->m_RenderTargetView) );
	if ( FAILED( hr ) )
	{
		LOG_ERROR(" Create the Render Target View - Failed \n ");
		return false;
	}

	backBuffer->Release();


	// Set the Render Target
	this->m_D3D11DevCon->OMSetRenderTargets( 1, &(this->m_RenderTargetView), this->m_DepthStencilView );

	return true;
}

void DXENGINE::MainLoop()
{
	MSG msg;
	ZeroMemory( &msg, sizeof( MSG ) );

	while( true )
	{
		if ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
		{
			if ( msg.message == WM_QUIT )
			{
				break;
			}
			else
			{
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
		}
		else
		{
			// Game LOGIC
			UpdateScene();
			DrawScene();
		}
	}
}

void DXENGINE::Cleanup()
{
	m_SwapChain->Release();
	m_D3D11Device->Release();
	m_D3D11DevCon->Release();
	m_RenderTargetView->Release();

	m_VS->Release();
	m_PS->Release();
	m_VS_Buffer->Release();
	m_PS_Buffer->Release();
	m_VertLayout->Release();

	m_SquareIndexBuffer->Release();
	m_SquareVertBuffer->Release();

	m_DepthStencilBuffer->Release();
	m_DepthStencilView->Release();
}

bool DXENGINE::InitScene()
{
	HRESULT hr;

	D3D11_INPUT_ELEMENT_DESC Layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	UINT numElements = ARRAYSIZE( Layout );

	this->VertexShader = ".\\..\\..\\shader\\VertexShader.hlsl";
	this->PixelShader = ".\\..\\..\\shader\\PixelShader.hlsl";
	this->Shader = ".\\..\\..\\shader\\Shader.fx";


	// Compile Shaders from shader file
	hr = D3DX11CompileFromFile( this->Shader, 0, 0, "VS", "vs_4_0", 0, 0, 0, &(this->m_VS_Buffer), 0, 0 );
	if ( FAILED( hr ) )
	{
		LOG_ERROR(" Compile From Vertex Shader file - Failed \n ");
		//return false;
	}
	else
	{
		LOG_INFO(" Compile From Vertex Shader file - Successed \n ");
	}

	hr = D3DX11CompileFromFile( this->Shader, 0, 0, "PS", "ps_4_0", 0, 0, 0, &(this->m_PS_Buffer), 0, 0 );
	if ( FAILED( hr ) )
	{
		LOG_ERROR(" Compile From Pixel Shader file - Failed \n ");
		//return false;
	}
	else
	{
		LOG_INFO(" Compile From Pixel Shader file - Successed \n ");
	}

	// Create the Shader Objects
	hr = this->m_D3D11Device->CreateVertexShader( this->m_VS_Buffer->GetBufferPointer(), this->m_VS_Buffer->GetBufferSize(), NULL, &(this->m_VS) );
	if ( FAILED( hr ) )
	{
		LOG_ERROR(" Create Vertex Shader file - Failed \n ");
		return false;
	}
	else
	{
		LOG_INFO(" Create Vertex Shader file - Successed \n ");
	}

	hr = this->m_D3D11Device->CreatePixelShader( this->m_PS_Buffer->GetBufferPointer(), this->m_PS_Buffer->GetBufferSize(), NULL, &(this->m_PS) );
	if ( FAILED( hr ) )
	{
		LOG_ERROR(" Create Pixel Shader file - Failed \n ");
		return false;
	}
	else
	{
		LOG_INFO(" Create Pixel Shader file - Successed \n ");
	}


	// Set Vertex and Pixel Shaders
	this->m_D3D11DevCon->VSSetShader( this->m_VS, 0, 0 );
	this->m_D3D11DevCon->PSSetShader( this->m_PS, 0, 0 );


	// Create the Vertex buffer
	Vertex v[] =
	{
		Vertex( -0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f ),
		Vertex( -0.5, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f ),
		Vertex( 0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f ),
		Vertex( 0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f ),
	};

	// Describe Vertex Buffer
	D3D11_BUFFER_DESC vertexBufferDesc;

	ZeroMemory( &vertexBufferDesc, sizeof( vertexBufferDesc ) );

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof( Vertex ) * 4;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexBufferData;

	ZeroMemory( &vertexBufferData, sizeof( vertexBufferData ) );

	vertexBufferData.pSysMem = v;

	hr = this->m_D3D11Device->CreateBuffer( &vertexBufferDesc, &vertexBufferData, &(this->m_SquareVertBuffer ) );
	if ( FAILED( hr ) )
	{
		LOG_ERROR(" Create Square Vertex Buffer - Failed \n ");
		return false;
	}
	else
	{
		LOG_INFO(" Create Square Vertex Buffer - Succssed \n ");
	}


	// Set the Vertex Buffer;
	UINT stride = sizeof( Vertex );
	UINT offset = 0;
	this->m_D3D11DevCon->IASetVertexBuffers( 0, 1, &(this->m_SquareVertBuffer), &stride, &offset );


	// Create the Index buffer

	DWORD indices_1[] =
	{
		0, 1, 2,
	};

	DWORD indices_2[] =
	{
		0, 2, 3,
	};


	// Describe the Index buffer
	D3D11_BUFFER_DESC indexBufferDesc;

	ZeroMemory( &indexBufferDesc, sizeof( indexBufferDesc ) );

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof( DWORD ) * 3;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData;

	iinitData.pSysMem = indices_1;

	hr = this->m_D3D11Device->CreateBuffer( &indexBufferDesc, &iinitData, &(this->m_SquareIndexBuffer) );
	this->m_D3D11DevCon->IASetIndexBuffer( this->m_SquareIndexBuffer, DXGI_FORMAT_R32_UINT, 0 );


	// 어떻게 해야 two Index Buffer를 넣을 수 있을까??
	iinitData.pSysMem = indices_2;

	hr = this->m_D3D11Device->CreateBuffer( &indexBufferDesc, &iinitData, &(this->m_SquareIndexBuffer) );
	this->m_D3D11DevCon->IASetIndexBuffer( this->m_SquareIndexBuffer, DXGI_FORMAT_R32_UINT, 1 );


	// Create the Input Layout
	this->m_D3D11Device->CreateInputLayout( Layout, numElements, this->m_VS_Buffer->GetBufferPointer(), this->m_VS_Buffer->GetBufferSize(), &(this->m_VertLayout) );


	// Set the Input Layout
	this->m_D3D11DevCon->IASetInputLayout( this->m_VertLayout );


	// Set Primitive Topology
	this->m_D3D11DevCon->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );


	// Create the Viewport
	D3D11_VIEWPORT viewport;

	ZeroMemory( &viewport, sizeof( D3D11_VIEWPORT) );

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)this->m_Width;
	viewport.Height = (float)this->m_Height;


	// Set the Viewport
	this->m_D3D11DevCon->RSSetViewports( 1, &viewport );
	return true;
}

bool DXENGINE::UpdateScene()
{
	return true;
}

bool DXENGINE::DrawScene()
{
	// Clear our Backbuffer
	float bgColor[4] = { ( 0.0f, 0.0f, 0.0f, 0.0f ) };
	this->m_D3D11DevCon->ClearRenderTargetView( this->m_RenderTargetView, bgColor );


	// Draw the triangle
	this->m_D3D11DevCon->DrawIndexed( 3, 0, 0 );
	this->m_D3D11DevCon->DrawIndexed( 3, 0, 1 );

	/*
	this->m_D3D11DevCon->DrawIndexed( 3, 0, indices_2 );
	*/

	// Present the Backbuffer to the screen
	this->m_SwapChain->Present( 0, 0 );

	return true;
}
