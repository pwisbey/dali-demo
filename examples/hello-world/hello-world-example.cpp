/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <dali-toolkit/dali-toolkit.h>
#include <dali/devel-api/adaptor-framework/tilt-sensor.h>
#include <dali/integration-api/debug.h>
#include <dali/devel-api/adaptor-framework/bitmap-loader.h>
#include <stdio.h>
using namespace Dali;

namespace
{
const char* TEXTURE_URL[] =
{
  DEMO_IMAGE_DIR "uv_checkerboard.jpg",
  DEMO_IMAGE_DIR "cube-border.png",//DEMO_IMAGE_DIR "tbcol.png",
  DEMO_IMAGE_DIR "baked-room.png"
};


const char* MODEL_URL[] =
{
  DEMO_MODEL_DIR "sphere.bin",
  DEMO_MODEL_DIR "cube.bin",
  DEMO_MODEL_DIR "vrroom.bin",
  //DEMO_MODEL_DIR "vrroom.bin",
};

const char* VERTEX_SHADER =
    "attribute highp vec3 aPosition;\n"
    "attribute highp vec3 aNormal;\n"
    "attribute highp vec2 aTexCoord;\n"
    "uniform highp mat4 uMvpMatrix;\n"
    "uniform highp mat3 uNormalMatrix;\n"
    //"uniform mediump vec3 uSize;\n"
    "varying highp vec3 vPosition;\n"
    "varying highp vec2 vTexCoord;\n"
    "varying highp vec3 vNormal;\n"
    "void main()\n"
    "{\n"
    "  gl_Position = uMvpMatrix * vec4(aPosition, 1.0 );\n"
    "  vPosition = aPosition;\n"
    "  vNormal = normalize( uNormalMatrix * aNormal );\n"
    "  vTexCoord = aTexCoord;\n"
    "  vTexCoord.y = 1.0 - vTexCoord.y;\n"
    "}\n"
    ;
const char* FRAGMENT_SHADER =
    "uniform sampler2D sTexture;\n"
    "varying highp vec3 vPosition;\n"
    "varying highp vec2 vTexCoord;\n"
    "varying highp vec3 vNormal;\n"
    "uniform mediump float uLightPwr;\n"
    "void main()\n"
    "{\n"
    "  mediump float dist = distance( vPosition, vec3( 0.0, 0.0, 0.0 ));\n"
    "  mediump float NdotL = max(dot(vNormal,normalize( vec3( 1.0, 1.0, -1.0 ) )),0.0);\n"
    "  mediump float fcolor = 3.0 * NdotL;\n//0.5 + uLightPwr * (1.0/(1.0 + (0.25*dist*dist))) * NdotL;\n"
    "  fcolor = 0.3 + fcolor;\n"
    "  gl_FragColor = texture2D( sTexture, vTexCoord ) * vec4( fcolor, fcolor, fcolor, 1.0 );\n"
    "}\n";
}
// This example shows how to create and display Hello World! using a simple TextActor
//
class HelloWorldController : public ConnectionTracker
{
public:

  HelloWorldController( Application& application )
  : mApplication( application )
  {
    // Connect to the Application's Init signal
    mApplication.InitSignal().Connect( this, &HelloWorldController::Create );
  }

  ~HelloWorldController()
  {
    // Nothing to do here;
  }

  // The Init signal is received once (only) during the Application lifetime
  void Create( Application& application )
  {
    // Get a handle to the stage
    Stage stage = Stage::GetCurrent();

    Stage::GetCurrent().GetRootLayer().SetBehavior( Layer::LAYER_3D );
    Stage::GetCurrent().GetRootLayer().SetDepthTestDisabled( false );
    application.SetViewMode( VR );

    CameraActor camera = Stage::GetCurrent().GetCameraActor();
    camera.SetPosition( 0.0f, 0.0f, 0.0f );

    mRenderer = CreateRenderer( MODEL_URL[0], TEXTURE_URL[0], FaceCullingMode::NONE, false, true );
    mRenderer.RegisterProperty( "uLightPwr", 30.0f );
    CreateActor( mRenderer, Vector3( 0.0f, 0.0f, 0.0f ), Vector3( 200.0f, 200.0f, 200.0f ));

    mRendererRoom = CreateRenderer( MODEL_URL[2], TEXTURE_URL[2], FaceCullingMode::NONE, true, true );
    mRendererRoom.RegisterProperty( "uLightPwr", 100.0f );
    Actor room = CreateActor( mRendererRoom, Vector3( 0.0f, 0.0f, 0.0f ), Vector3( 50.0f, 15.0f, 50.0f ));
    room.RotateBy( Degree(270), Vector3( 0.0f, 0.0f, 1.0f));
    room.RotateBy( Degree(45), Vector3( 0.0f, 1.0f, 0.0f));

    float IPD = 0.0635f;

    mRendererMonkey = CreateRenderer( MODEL_URL[1], TEXTURE_URL[1], FaceCullingMode::NONE, true );
    mRendererMonkey.RegisterProperty( "uLightPwr", 2.0f );
    Actor monkey = CreateActor( mRendererMonkey, Vector3( 0.0f, 0.0f, IPD*30.0f+1.0f ), Vector3( 1.0f, 1.0f, 4.0f ));
    monkey.SetOrientation( Radian( Degree(-45.0f)), Vector3( 1.0f, 1.0f, 0.0f) );

    Actor monkey2 = CreateActor( mRendererMonkey, Vector3( -5.0f, -5.0f, IPD*30.0f+10 ), Vector3( 1.0f, 1.0f, 1.0f ));
    monkey2.SetOrientation( Radian( Degree(-90.0f)), Vector3( 1.0f, 1.0f, 0.0f) );

    mAnimation = Animation::New( 20.0f );
    CameraActor actor = Stage::GetCurrent().GetCameraActor();
    Quaternion quat( Radian( Degree( 359.0f )), Vector3( 1.0f, 1.0f, 1.0f ) );
    Quaternion quat2( Radian( Degree( 1.0f )), Vector3( 1.0f, 0.0f, 0.0f ) );

    Vector3 posz( 0.0f, 0.0f, -20 );
    mAnimation.SetLooping( true );
    //mAnimation.AnimateBy( Property( monkey, Actor::Property::POSITION ), posz, AlphaFunction::BOUNCE );
    mAnimation.AnimateBy( Property( monkey, Actor::Property::ORIENTATION ), quat, AlphaFunction::LINEAR );
    mAnimation.AnimateBy( Property( monkey2, Actor::Property::ORIENTATION ), quat, AlphaFunction::LINEAR );
    //mAnimation.AnimateBy( Property( camera, Actor::Property::ORIENTATION ), quat2, AlphaFunction::LINEAR );
    mAnimation.Play();

    /*
    TiltSensor mSensor = TiltSensor::Get();
    if( mSensor.Enable() )
    {
      mSensor.TiltedSignal().Connect( this, &HelloWorldController::OnTilted );
    }
    */


    //Quaternion quat2 = Quaternion(-0.015448, 0.240648, 0.003831, 0.970482 );
    //Stage::GetCurrent().GetCameraActor().SetOrientation( quat2.Normalized() );

    // Respond to a click anywhere on the stage
    //monkey.TouchSignal().Connect( this, &HelloWorldController::OnTouch );
    stage.GetRootLayer().TouchSignal().Connect( this, &HelloWorldController::OnTouch );
  }

  void OnTilted(const TiltSensor& sensor)
  {

    //DALI_LOG_ERROR(")
    //sensor.GetPitch();
    //float yaw = sensor.GetYaw();
    //float roll = sensor.GetRoll();
    Quaternion quat = sensor.GetRotation();

    //Stage::Ge
    //Actor actor = Stage::GetCurrent().GetLayer( 0 ).GetChildAt( 0 );
    DALI_LOG_ERROR("ADAM: %f, %f, %f, %f",
                   quat.AsVector().x,
                   quat.AsVector().y,
                   quat.AsVector().z,
                   quat.AsVector().w
                   );

    //Quaternion quat2( quat.AsVector().y, quat.AsVector().x, quat.AsVector().z, quat.AsVector().w );
    //quat2.mVector.y = quat.AsVector().y;
    Stage::GetCurrent().GetCameraActor().SetOrientation( quat.Normalized() );
    //actor.SetOrientation( quat );

  }


  bool OnTouch( Actor actor, const TouchData& touch )
  {
    puts("On touch");
    fflush(stdout);

    // quit the application
//    mApplication.Quit();
    return true;
  }

  int LoadModel( const char* filename, std::vector<char>& outdata )
  {
    FILE* fin = fopen( filename, "rb" );
    puts(filename);
    fflush(stdout);
    fseek( fin, 0, SEEK_END );
    outdata.resize( ftell( fin ));
    fseek( fin, 0, SEEK_SET );
    int value = fread( outdata.data(), 1, outdata.size(), fin );
    fclose(fin);
    return value;
  }

  Renderer CreateRenderer( const char* model, const char* textureName, FaceCullingMode::Type mode, bool depthWrite = true, bool depthTest = true )
  {
    std::vector<char> modelData;
    LoadModel( model, modelData );

    // load texture
    BitmapLoader loader = BitmapLoader::New( textureName, ImageDimensions( 1024, 1024 ) );
    loader.Load();
    Dali::PixelData pixelData = loader.GetPixelData();
    Texture texture = Texture::New( TextureType::TEXTURE_2D,
                                    pixelData.GetPixelFormat(),
                                    pixelData.GetWidth(),
                                    pixelData.GetHeight() );
    texture.Upload( pixelData );
    TextureSet textureSet = TextureSet::New();
    textureSet.SetTexture( 0, texture );
    Sampler sampler = Sampler::New();
    sampler.SetWrapMode( WrapMode::REPEAT, WrapMode::REPEAT, WrapMode::REPEAT );
    textureSet.SetSampler( 0, sampler );
    // create renderer
    if( !mShader )
    {
      mShader = Shader::New( VERTEX_SHADER, FRAGMENT_SHADER );
    }
    Geometry geometry = Geometry::New();
    geometry.SetType( Geometry::TRIANGLES );
    Property::Map format;
    format["aPosition"] = Dali::Property::VECTOR3;
    format["aNormal"] = Dali::Property::VECTOR3;
    format["aTexCoord"] = Dali::Property::VECTOR2;
    PropertyBuffer vertexBuffer = PropertyBuffer::New( format );
    int numverts = (modelData.size() - 4)/(sizeof(float)*8);
    vertexBuffer.SetData( &modelData[4], numverts );
    geometry.AddVertexBuffer( vertexBuffer );

    Renderer renderer = Renderer::New( geometry, mShader );
    renderer.SetTextures( textureSet );
    renderer.SetProperty( Renderer::Property::FACE_CULLING_MODE, mode );
    renderer.SetProperty( Renderer::Property::DEPTH_WRITE_MODE, depthWrite ?  DepthWriteMode::ON : DepthWriteMode::OFF );
    renderer.SetProperty( Renderer::Property::DEPTH_FUNCTION, DepthFunction::LESS_EQUAL );
    renderer.SetProperty( Renderer::Property::DEPTH_TEST_MODE, depthTest ? DepthTestMode::ON : DepthTestMode::OFF );


    return renderer;

  }

  Actor CreateActor( Renderer renderer, Vector3 position, Vector3 scale )
  {
    Actor actor = Actor::New();
    actor.SetPosition( position );
    actor.SetSize( scale );
    actor.SetScale( scale );
    actor.SetAnchorPoint( AnchorPoint::CENTER );
    actor.SetParentOrigin( ParentOrigin::CENTER );
    actor.AddRenderer( renderer );
    Stage::GetCurrent().GetRootLayer().Add( actor );
    return actor;
  }

private:
  Application&  mApplication;
  Renderer mRenderer;
  Renderer mRendererMonkey;
  Renderer mRendererRoom;

  Shader mShader;
  TiltSensor mSensor;
  Animation mAnimation;

};

void RunTest( Application& application )
{
  HelloWorldController test( application );

  application.MainLoop();
}

// Entry point for Linux & Tizen applications
//
int DALI_EXPORT_API main( int argc, char **argv )
{
  Application application = Application::New( &argc, &argv );

  RunTest( application );

  return 0;
}

