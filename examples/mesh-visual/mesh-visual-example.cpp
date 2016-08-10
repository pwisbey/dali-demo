#include <dali-toolkit/dali-toolkit.h>
#include <dali/public-api/object/property-map.h>
#include "mesh-visual-shaders.h"
#include "shared/utility.h"

using namespace Dali;
using namespace Dali::Toolkit;

namespace
{
  //Keeps information about each model for access.
  struct Model
  {
    Control control; // Control housing the mesh visual of the model.
    Vector2 rotation; // Keeps track of rotation about x and y axis for manual rotation.
    Animation rotationAnimation; // Automatically rotates when left alone.
  };

  //Files for meshes
  const char * const MODEL_FILE_TABLE[] =
  {
      DEMO_MODEL_DIR "Dino.obj",
      DEMO_MODEL_DIR "ToyRobot-Metal.obj",
      DEMO_MODEL_DIR "Toyrobot-Plastic.obj"
  };

  const char * const MATERIAL_FILE_TABLE[] =
  {
      DEMO_MODEL_DIR "Dino.mtl",
      DEMO_MODEL_DIR "ToyRobot-Metal.mtl",
      DEMO_MODEL_DIR "Toyrobot-Plastic.mtl"
  };

  const char * const TEXTURES_PATH( DEMO_IMAGE_DIR "" );

  //Possible shading modes.
  MeshVisual::ShadingMode::Value SHADING_MODE_TABLE[] =
  {
    MeshVisual::ShadingMode::TEXTURED_WITH_DETAILED_SPECULAR_LIGHTING,
    MeshVisual::ShadingMode::TEXTURED_WITH_SPECULAR_LIGHTING,
    MeshVisual::ShadingMode::TEXTURELESS_WITH_DIFFUSE_LIGHTING
  };

  const float X_ROTATION_DISPLACEMENT_FACTOR = 60.0f;
  const float Y_ROTATION_DISPLACEMENT_FACTOR = 60.0f;
  const float MODEL_SCALE =                    0.75f;
  const float LIGHT_SCALE =                    0.15f;
  const float BUTTONS_OFFSET_BOTTOM =          0.9f;
  const int   NUM_MESHES =                     1;//TODOVR

  //Used to identify actors.
  const int MODEL_TAG = 0;
  const int LIGHT_TAG = 1;
  const int LAYER_TAG = 2;

} //End namespace

class MeshVisualController : public ConnectionTracker
{
public:

  MeshVisualController( Application& application )
  : mApplication( application ),   //Store handle to the application.
    mModelIndex( 2 ),              //Start with metal robot.
    mShadingModeIndex( 1 ),        //Start with textured with detailed specular lighting.
    mTag( -1 ),                    //Non-valid default, which will get set to a correct value when used.
    mSelectedModelIndex( -1 ),     //Non-valid default, which will get set to a correct value when used.
    mPaused( false ),              //Animations play by default.
    mLightFixed( true )            //The light is fixed by default.
  {
    // Connect to the Application's Init signal
    mApplication.InitSignal().Connect( this, &MeshVisualController::Create );
  }

  ~MeshVisualController()
  {
  }

  // The Init signal is received once (only) during the Application lifetime
  void Create( Application& application )
  {
    //todor
    application.SetViewMode( VR );

    // Get a handle to the stage
    Stage stage = Stage::GetCurrent();
    stage.SetBackgroundColor( Vector4( 0.0, 0.5, 1.0, 1.0 ) );
    application.SetViewMode( Dali::VR );
    //application.SetStereoBase( 105.0f );
    //Set up layer to place objects on.
    Layer baseLayer = Layer::New();
    baseLayer.SetParentOrigin( ParentOrigin::CENTER );
    baseLayer.SetAnchorPoint( AnchorPoint::CENTER );
    baseLayer.SetResizePolicy( ResizePolicy::FILL_TO_PARENT, Dimension::ALL_DIMENSIONS );
    baseLayer.SetBehavior( Layer::LAYER_2D ); //We use a 2D layer as this is closer to UI work than full 3D scene creation.
    baseLayer.SetDepthTestDisabled( false ); //Enable depth testing, as otherwise the 2D layer would not do so.
    baseLayer.RegisterProperty( "Tag", LAYER_TAG ); //Used to differentiate between different kinds of actor.
    baseLayer.TouchedSignal().Connect( this, &MeshVisualController::OnTouch );
    stage.Add( baseLayer );



    const float   CUBE_WIDTH_SCALE( 2.0f );                   ///< The width (and height + depth) of the main and reflection cubes.
    const char * const CUBE_TEXTURE( DEMO_IMAGE_DIR "people-medium-1.jpg" );
    const Vector4 CUBE_COLOR( 1.0f, 1.0f, 1.0f, 1.0f );        ///< White.
    // Main cube:
    // Make the demo scalable with different resolutions by basing
    // the cube size on a percentage of the stage size.
    float scaleSize( std::min( stage.GetSize().width, stage.GetSize().height ) );
    float cubeWidth( scaleSize * CUBE_WIDTH_SCALE );
    Vector3 cubeSize( cubeWidth, cubeWidth, cubeWidth );
    // Create the geometry for the cube, and the texture.
    Geometry cubeGeometry = CreateCubeVertices( Vector3::ONE, false );
    TextureSet cubeTextureSet = CreateTextureSet( CUBE_TEXTURE );
    // Create the cube object and add it.
    // Note: The cube is anchored around its base for animation purposes, so the position can be zero.
    Toolkit::Control container = Toolkit::Control::New();
    container.SetAnchorPoint( AnchorPoint::BOTTOM_CENTER );
    container.SetParentOrigin( ParentOrigin::BOTTOM_CENTER );
    container.SetSize( cubeSize );
    container.SetPosition( 0.0f, 0.0f, 960.0f );
    container.SetResizePolicy( ResizePolicy::FIXED, Dimension::ALL_DIMENSIONS );
    // Create a renderer from the geometry and add the texture.
    Renderer renderer = CreateRenderer( cubeGeometry, cubeSize, true, CUBE_COLOR );
    renderer.SetTextures( cubeTextureSet );
    // Setup the renderer properties:
    // We are writing to the color buffer & culling back faces.
    renderer.SetProperty( Renderer::Property::WRITE_TO_COLOR_BUFFER, true );
    renderer.SetProperty( Renderer::Property::FACE_CULLING_MODE, FaceCullingMode::FRONT );
    // No stencil is used for the main cube.
    renderer.SetProperty( Renderer::Property::STENCIL_MODE, StencilMode::OFF );
    // We do need to write to the depth buffer as other objects need to appear underneath this cube.
    renderer.SetProperty( Renderer::Property::DEPTH_WRITE_MODE, DepthWriteMode::OFF );
    // We do not need to test the depth buffer as we are culling the back faces.
    renderer.SetProperty( Renderer::Property::DEPTH_TEST_MODE, DepthTestMode::OFF );
    // This object must be rendered 1st.
    //renderer.SetProperty( Renderer::Property::DEPTH_INDEX, 0 * DEPTH_INDEX_GRANULARITY );
    container.AddRenderer( renderer );
    baseLayer.Add( container );



    //Place models on the scene.
    SetupModels( baseLayer );

    //Place buttons on the scene.
    SetupButtons( baseLayer );

    //Add a light to the scene.
    SetupLight( baseLayer );

    //Allow for exiting of the application via key presses.
    stage.KeyEventSignal().Connect( this, &MeshVisualController::OnKeyEvent );
  }

  //Loads and adds the models to the scene, inside containers for hit detection.
  void SetupModels( Layer layer )
  {
    //Add containers to house each renderer-holding-actor.
    for( int i = 0; i < NUM_MESHES; i++ )
    {
      mContainers[i] = Actor::New();
      mContainers[i].SetResizePolicy( ResizePolicy::SIZE_RELATIVE_TO_PARENT, Dimension::ALL_DIMENSIONS );
      mContainers[i].RegisterProperty( "Tag", MODEL_TAG ); //Used to differentiate between different kinds of actor.
      mContainers[i].RegisterProperty( "Model", Property::Value( i ) ); //Used to index into the model.
      mContainers[i].TouchedSignal().Connect( this, &MeshVisualController::OnTouch );
      layer.Add( mContainers[i] );
    }

    //Position each container individually on screen

    //Main, central model
    mContainers[0].SetSizeModeFactor( Vector3( MODEL_SCALE, MODEL_SCALE, 0.0f ) );
    mContainers[0].SetParentOrigin( ParentOrigin::CENTER );
    mContainers[0].SetAnchorPoint( AnchorPoint::CENTER );

    //TODOVR
#if 0
    //Top left model
    mContainers[1].SetSizeModeFactor( Vector3( MODEL_SCALE / 3.0f, MODEL_SCALE / 3.0f, 0.0f ) );
    mContainers[1].SetParentOrigin( Vector3( 0.05, 0.03, 0.5 ) ); //Offset from top left
    mContainers[1].SetAnchorPoint( AnchorPoint::TOP_LEFT );
#endif

    //Set up models
    for( int i = 0; i < NUM_MESHES; i++ )
    {
      //Create control to display model
      Control control = Control::New();
      control.SetResizePolicy( ResizePolicy::FILL_TO_PARENT, Dimension::ALL_DIMENSIONS );
      control.SetParentOrigin( ParentOrigin::CENTER );
      control.SetAnchorPoint( AnchorPoint::CENTER );
      mContainers[i].Add( control );

      //Make model spin to demonstrate 3D
      Animation rotationAnimation = Animation::New( 15.0f );
      float spin = i % 2 == 0 ? 1.0f : -1.0f; //Make actors spin in different directions to better show independence.
      rotationAnimation.AnimateBy( Property( control, Actor::Property::ORIENTATION ),
                                   Quaternion( Degree( 0.0f ), Degree( spin * 360.0f ), Degree( 0.0f ) ) );
      rotationAnimation.SetLooping( true );
      rotationAnimation.Play();

      //Store model information in corresponding structs.
      mModels[i].control = control;
      mModels[i].rotation.x = 0.0f;
      mModels[i].rotation.y = 0.0f;
      mModels[i].rotationAnimation = rotationAnimation;
    }

    //Calling this sets the model in the controls.
    ReloadModel();
  }

  //Place the various buttons on the bottom of the screen, with title labels where necessary.
  void SetupButtons( Layer layer )
  {
#if 1
    //Text label title for changing model or shader.
    TextLabel changeTitleLabel = TextLabel::New( "Switch" );
    changeTitleLabel.SetResizePolicy( ResizePolicy::USE_NATURAL_SIZE, Dimension::ALL_DIMENSIONS );
    changeTitleLabel.SetProperty( TextLabel::Property::UNDERLINE, "{\"thickness\":\"2.0\"}" );
    changeTitleLabel.SetParentOrigin( Vector3( 0.2, BUTTONS_OFFSET_BOTTOM, 0.5 ) );
    changeTitleLabel.SetAnchorPoint( AnchorPoint::BOTTOM_CENTER );
    layer.Add( changeTitleLabel );
#endif
    //Create button for model changing
    PushButton modelButton = Toolkit::PushButton::New();
    modelButton.SetResizePolicy( ResizePolicy::USE_NATURAL_SIZE, Dimension::ALL_DIMENSIONS );
    modelButton.ClickedSignal().Connect( this, &MeshVisualController::OnChangeModelClicked );
    modelButton.SetParentOrigin( ParentOrigin::BOTTOM_CENTER );
    modelButton.SetAnchorPoint( AnchorPoint::TOP_RIGHT );
    modelButton.SetLabelText( "Model" );
    changeTitleLabel.Add( modelButton );

#if 0
    //Create button for shader changing
    PushButton shaderButton = Toolkit::PushButton::New();
    shaderButton.SetResizePolicy( ResizePolicy::USE_NATURAL_SIZE, Dimension::ALL_DIMENSIONS );
    shaderButton.ClickedSignal().Connect( this, &MeshVisualController::OnChangeShaderClicked );
    shaderButton.SetParentOrigin( ParentOrigin::BOTTOM_CENTER );
    shaderButton.SetAnchorPoint( AnchorPoint::TOP_LEFT );
    shaderButton.SetLabelText( "Shader" );
    changeTitleLabel.Add( shaderButton );

    //Create button for pausing animations
    PushButton pauseButton = Toolkit::PushButton::New();
    pauseButton.SetResizePolicy( ResizePolicy::USE_NATURAL_SIZE, Dimension::ALL_DIMENSIONS );
    pauseButton.ClickedSignal().Connect( this, &MeshVisualController::OnPauseClicked );
    pauseButton.SetParentOrigin( Vector3( 0.5, BUTTONS_OFFSET_BOTTOM, 0.5 ) );
    pauseButton.SetAnchorPoint( AnchorPoint::TOP_CENTER );
    pauseButton.SetLabelText( " || " );
    layer.Add( pauseButton );
#endif

    //Text label title for light position mode.
    TextLabel lightTitleLabel = TextLabel::New( "Light Position" );
    lightTitleLabel.SetResizePolicy( ResizePolicy::USE_NATURAL_SIZE, Dimension::ALL_DIMENSIONS );
    lightTitleLabel.SetProperty( TextLabel::Property::UNDERLINE, "{\"thickness\":\"2.0\"}" );
    lightTitleLabel.SetParentOrigin( Vector3( 0.8, BUTTONS_OFFSET_BOTTOM, 0.5 ) );
    lightTitleLabel.SetAnchorPoint( AnchorPoint::BOTTOM_CENTER );
    layer.Add( lightTitleLabel );

#if 0
    //Create button for switching between manual and fixed light position.
    PushButton lightButton = Toolkit::PushButton::New();
    lightButton.SetResizePolicy( ResizePolicy::USE_NATURAL_SIZE, Dimension::ALL_DIMENSIONS );
    lightButton.ClickedSignal().Connect( this, &MeshVisualController::OnChangeLightModeClicked );
    lightButton.SetParentOrigin( ParentOrigin::BOTTOM_CENTER );
    lightButton.SetAnchorPoint( AnchorPoint::TOP_CENTER );
    lightButton.SetLabelText( "FIXED" );
    lightTitleLabel.Add( lightButton );
#endif
  }

  //Add a point light source the the scene, on a layer above the first.
  void SetupLight( Layer baseLayer )
  {
    //Create control to act as light source of scene.
    mLightSource = Control::New();
    mLightSource.RegisterProperty( "Tag", LIGHT_TAG );

    //Set size of control based on screen dimensions.
    Stage stage = Stage::GetCurrent();
    if( stage.GetSize().width < stage.GetSize().height )
    {
      //Scale to width.
      mLightSource.SetResizePolicy( ResizePolicy::SIZE_RELATIVE_TO_PARENT, Dimension::WIDTH );
      mLightSource.SetResizePolicy( ResizePolicy::DIMENSION_DEPENDENCY, Dimension::HEIGHT );
      mLightSource.SetSizeModeFactor( Vector3( LIGHT_SCALE, 0.0f, 0.0f ) );
    }
    else
    {
      //Scale to height.
      mLightSource.SetResizePolicy( ResizePolicy::SIZE_RELATIVE_TO_PARENT, Dimension::HEIGHT );
      mLightSource.SetResizePolicy( ResizePolicy::DIMENSION_DEPENDENCY, Dimension::WIDTH );
      mLightSource.SetSizeModeFactor( Vector3( 0.0f, LIGHT_SCALE, 0.0f ) );
    }

    //Set position relative to top left, as the light source property is also relative to the top left.
    mLightSource.SetParentOrigin( ParentOrigin::TOP_LEFT );
    mLightSource.SetAnchorPoint( AnchorPoint::CENTER );
    mLightSource.SetPosition( Stage::GetCurrent().GetSize().x * 0.85f, Stage::GetCurrent().GetSize().y * 0.125 );

    //Supply an image to represent the light.
    Property::Map lightMap;
    lightMap.Insert( Visual::Property::TYPE, Visual::IMAGE );
    lightMap.Insert( ImageVisual::Property::URL, DEMO_IMAGE_DIR "light-icon.png" );
    mLightSource.SetProperty( Control::Property::BACKGROUND, Property::Value( lightMap ) );

    //Connect to touch signal for dragging.
    mLightSource.TouchedSignal().Connect( this, &MeshVisualController::OnTouch );

    //Place the light source on a layer above the base, so that it is rendered above everything else.
    Layer upperLayer = Layer::New();
    upperLayer.SetResizePolicy( ResizePolicy::FILL_TO_PARENT, Dimension::ALL_DIMENSIONS );
    upperLayer.SetParentOrigin( ParentOrigin::CENTER );
    upperLayer.SetAnchorPoint( AnchorPoint::CENTER );

    baseLayer.Add( upperLayer );
    upperLayer.Add( mLightSource );

    //Decide which light to use to begin with.
    if( mLightFixed )
    {
      UseFixedLight();
    }
    else
    {
      UseManualLight();
    }
  }

  //Updates the displayed models to account for parameter changes.
  void ReloadModel()
  {
    //Create mesh property map
    Property::Map map;
    map.Insert( Visual::Property::TYPE,  Visual::MESH );
    map.Insert( MeshVisual::Property::OBJECT_URL, MODEL_FILE_TABLE[mModelIndex] );
    map.Insert( MeshVisual::Property::MATERIAL_URL, MATERIAL_FILE_TABLE[mModelIndex] );
    map.Insert( MeshVisual::Property::TEXTURES_PATH, TEXTURES_PATH );
    map.Insert( MeshVisual::Property::SHADING_MODE, SHADING_MODE_TABLE[mShadingModeIndex] );

    //Set the two controls to use the mesh
    for( int i = 0; i < NUM_MESHES; i++ )
    {
      mModels[i].control.SetProperty( Control::Property::BACKGROUND, Property::Value( map ) );
    }
  }

  void UseFixedLight()
  {
    //Hide draggable source
    mLightSource.SetVisible( false );

    //Use stage dimensions to place light at center, offset outwards in z axis.
    Stage stage = Stage::GetCurrent();
    float width = stage.GetSize().width;
    float height = stage.GetSize().height;
    Vector3 lightPosition = Vector3( width / 2.0f, height / 2.0f, std::max( width, height ) * 5.0f );

    //Set global light position
    for( int i = 0; i < NUM_MESHES; ++i )
    {
      mModels[i].control.RegisterProperty( "lightPosition", lightPosition, Property::ANIMATABLE );
    }
  }

  void UseManualLight()
  {
    //Show draggable source
    mLightSource.SetVisible( true );

    //Update to switch light position to that off the source.
    UpdateLight();
  }

  //Updates the light position for each model to account for changes in the source on screen.
  void UpdateLight()
  {
    //Set light position to the x and y of the light control, offset out of the screen.
    Vector3 controlPosition = mLightSource.GetCurrentPosition();
    Vector3 lightPosition = Vector3( controlPosition.x, controlPosition.y, Stage::GetCurrent().GetSize().x / 2.0f );

    for( int i = 0; i < NUM_MESHES; ++i )
    {
      mModels[i].control.RegisterProperty( "lightPosition", lightPosition, Property::ANIMATABLE );
    }
  }

  //If the light source is touched, move it by dragging it.
  //If a model is touched, rotate it by panning around.
  bool OnTouch( Actor actor, const TouchEvent& event )
  {
    //Get primary touch point.
    const Dali::TouchPoint& point = event.GetPoint( 0 );

    switch( point.state )
    {
      case TouchPoint::Down:
      {
        //Determine what was touched.
        actor.GetProperty( actor.GetPropertyIndex( "Tag" ) ).Get( mTag );

        if( mTag == MODEL_TAG )
        {
          //Find out which model has been selected
          actor.GetProperty( actor.GetPropertyIndex( "Model" ) ).Get( mSelectedModelIndex );

          //Pause current animation, as the touch gesture will be used to manually rotate the model
          mModels[mSelectedModelIndex].rotationAnimation.Pause();

          //Store start points.
          mPanStart = point.screen;
          mRotationStart = mModels[mSelectedModelIndex].rotation;
        }

        break;
      }
      case TouchPoint::Motion:
      {
        //Switch on the kind of actor we're interacting with.
        switch( mTag )
        {
          case MODEL_TAG: //Rotate model
          {
            //Calculate displacement and corresponding rotation.
            Vector2 displacement = point.screen - mPanStart;
            mModels[mSelectedModelIndex].rotation = Vector2( mRotationStart.x - displacement.y / Y_ROTATION_DISPLACEMENT_FACTOR,   // Y displacement rotates around X axis
                                                             mRotationStart.y + displacement.x / X_ROTATION_DISPLACEMENT_FACTOR ); // X displacement rotates around Y axis
            Quaternion rotation = Quaternion( Radian( mModels[mSelectedModelIndex].rotation.x ), Vector3::XAXIS) *
                                  Quaternion( Radian( mModels[mSelectedModelIndex].rotation.y ), Vector3::YAXIS);

            //Apply rotation.
            mModels[mSelectedModelIndex].control.SetOrientation( rotation );

            break;
          }
          case LIGHT_TAG: //Drag light
          {
            //Set light source to new position and update the models accordingly.
            mLightSource.SetPosition( Vector3( point.screen ) );
            UpdateLight();

            break;
          }
        }

        break;
      }
      case TouchPoint::Interrupted: //Same as finished.
      case TouchPoint::Finished:
      {
        if( mTag == MODEL_TAG )
        {
          //Return to automatic animation
          if( !mPaused )
          {
            mModels[mSelectedModelIndex].rotationAnimation.Play();
          }
        }

        break;
      }
      default:
      {
        //Other touch states do nothing.
        break;
      }
    }

    return true;
  }

  //Cycle through the list of models.
  bool OnChangeModelClicked( Toolkit::Button button )
  {
    ++mModelIndex %= 3;

    ReloadModel();

    return true;
  }

  //Cycle through the list of shaders.
  bool OnChangeShaderClicked( Toolkit::Button button )
  {
    ++mShadingModeIndex %= 3;

    ReloadModel();

    return true;
  }

  //Pause all animations, and keep them paused even after user panning.
  //This button is a toggle, so pressing again will start the animations again.
  bool OnPauseClicked( Toolkit::Button button )
  {
    //Toggle pause state.
    mPaused = !mPaused;

    //If we wish to pause animations, do so and keep them paused.
    if( mPaused )
    {
      for( int i = 0; i < NUM_MESHES ; ++i )
      {
        mModels[i].rotationAnimation.Pause();
      }

      button.SetLabelText( " > " );
    }
    else //Unpause all animations again.
    {
      for( int i = 0; i < NUM_MESHES ; ++i )
      {
        mModels[i].rotationAnimation.Play();
      }

      button.SetLabelText( " || " );
    }

    return true;
  }

  //Switch between a fixed light source in front of the screen, and a light source the user can drag around.
  bool OnChangeLightModeClicked( Toolkit::Button button )
  {
    //Toggle state.
    mLightFixed = !mLightFixed;

    if( mLightFixed )
    {
      UseFixedLight();

      button.SetLabelText( "FIXED" );
    }
    else
    {
      UseManualLight();

      button.SetLabelText( "MANUAL" );
    }

    return true;
  }

  //If escape or the back button is pressed, quit the application (and return to the launcher)
  void OnKeyEvent( const KeyEvent& event )
  {
    if( event.state == KeyEvent::Down )
    {
      if( IsKey( event, DALI_KEY_ESCAPE) || IsKey( event, DALI_KEY_BACK ) )
      {
        mApplication.Quit();
      }
    }
  }

private:

  /**
   * @brief Struct to store the position, normal and texture coordinates of a single vertex.
   */
  struct TexturedVertex
  {
    Vector3 position;
    Vector3 normal;
    Vector2 textureCoord;
  };

  /**
   * @brief Creates a geometry object from vertices and indices.
   * @param[in] vertices The object vertices
   * @param[in] indices The object indices
   * @return A geometry object
   */
  Geometry CreateTexturedGeometry( Vector<TexturedVertex>& vertices, Vector<unsigned short>& indices )
  {
    // Vertices
    Property::Map vertexFormat;
    vertexFormat[POSITION] = Property::VECTOR3;
    vertexFormat[NORMAL] =   Property::VECTOR3;
    vertexFormat[TEXTURE] =  Property::VECTOR2;

    PropertyBuffer surfaceVertices = PropertyBuffer::New( vertexFormat );
    surfaceVertices.SetData( &vertices[0u], vertices.Size() );

    Geometry geometry = Geometry::New();
    geometry.AddVertexBuffer( surfaceVertices );

    // Indices for triangle formulation
    geometry.SetIndexBuffer( &indices[0u], indices.Size() );
    return geometry;
  }

  /**
   * @brief Creates a renderer from a geometry object.
   * @param[in] geometry The geometry to use
   * @param[in] dimensions The dimensions (will be passed in to the shader)
   * @param[in] textured Set to true to use the texture versions of the shaders
   * @param[in] color The base color for the renderer
   * @return A renderer object
   */
  Renderer CreateRenderer( Geometry geometry, Vector3 dimensions, bool textured, Vector4 color )
  {
    Stage stage = Stage::GetCurrent();
    Shader shader;

    if( textured )
    {
      shader = Shader::New( VERTEX_SHADER_TEXTURED, FRAGMENT_SHADER_TEXTURED );
    }
    else
    {
      shader = Shader::New( VERTEX_SHADER, FRAGMENT_SHADER );
    }

    // Here we modify the light position based on half the stage size as a pre-calculation step.
    // This avoids the work having to be done in the shader.
    shader.RegisterProperty( LIGHT_POSITION_UNIFORM_NAME, Vector3( -stage.GetSize().width / 2.0f, -stage.GetSize().width / 2.0f, 1000.0f ) );
    shader.RegisterProperty( COLOR_UNIFORM_NAME, color );
    shader.RegisterProperty( OBJECT_DIMENSIONS_UNIFORM_NAME, dimensions );

    return Renderer::New( geometry, shader );
  }

  /**
   * @brief Helper method to create a TextureSet from an image URL.
   * @param[in] url An image URL
   * @return A TextureSet object
   */
  TextureSet CreateTextureSet( const char* url )
  {
    TextureSet textureSet = TextureSet::New();

    if( textureSet )
    {
      Texture texture = DemoHelper::LoadTexture( url );
      if( texture )
      {
        textureSet.SetTexture( 0u, texture );
      }
    }

    return textureSet;
  }

  /**
   * @brief Creates a geometry object for a cube (or cuboid).
   * @param[in] dimensions The desired cube dimensions
   * @param[in] reflectVerticalUVs Set to True to force the UVs to be vertically flipped
   * @return A Geometry object
   */
  Geometry CreateCubeVertices( Vector3 dimensions, bool reflectVerticalUVs )
  {
    Vector<TexturedVertex> vertices;
    Vector<unsigned short> indices;
    int vertexIndex = 0u; // Tracks progress through vertices.
    float scaledX = 0.5f * dimensions.x;
    float scaledY = 0.5f * dimensions.y;
    float scaledZ = 0.5f * dimensions.z;
    float verticalTextureCoord = reflectVerticalUVs ? 0.0f : 1.0f;

    vertices.Resize( 4u * 6u ); // 4 vertices x 6 faces

    Vector<Vector3> positions;  // Stores vertex positions, which are shared between vertexes at the same position but with a different normal.
    positions.Resize( 8u );
    Vector<Vector3> normals;    // Stores normals, which are shared between vertexes of the same face.
    normals.Resize( 6u );

    positions[0] = Vector3( -scaledX,  scaledY, -scaledZ );
    positions[1] = Vector3(  scaledX,  scaledY, -scaledZ );
    positions[2] = Vector3(  scaledX,  scaledY,  scaledZ );
    positions[3] = Vector3( -scaledX,  scaledY,  scaledZ );
    positions[4] = Vector3( -scaledX, -scaledY, -scaledZ );
    positions[5] = Vector3(  scaledX, -scaledY, -scaledZ );
    positions[6] = Vector3(  scaledX, -scaledY,  scaledZ );
    positions[7] = Vector3( -scaledX, -scaledY,  scaledZ );

    normals[0] = Vector3(  0,  1,  0 );
    normals[1] = Vector3(  0,  0, -1 );
    normals[2] = Vector3(  1,  0,  0 );
    normals[3] = Vector3(  0,  0,  1 );
    normals[4] = Vector3( -1,  0,  0 );
    normals[5] = Vector3(  0, -1,  0 );

    // Top face, upward normals.
    for( int i = 0; i < 4; ++i, ++vertexIndex )
    {
      vertices[vertexIndex].position = positions[i];
      vertices[vertexIndex].normal = normals[0];
      // The below logic forms the correct U/V pairs for a quad when "i" goes from 0 to 3.
      vertices[vertexIndex].textureCoord = Vector2( ( i == 1 || i == 2 ) ? 1.0f : 0.0f, ( i == 2 || i == 3 ) ? 1.0f : 0.0f );
    }

    // Top face, outward normals.
    for( int i = 0; i < 4; ++i, vertexIndex += 2 )
    {
      vertices[vertexIndex].position = positions[i];
      vertices[vertexIndex].normal = normals[i + 1];

      if( i == 3 )
      {
        // End, so loop around.
        vertices[vertexIndex + 1].position = positions[0];
      }
      else
      {
        vertices[vertexIndex + 1].position = positions[i + 1];
      }
      vertices[vertexIndex + 1].normal = normals[i + 1];

      vertices[vertexIndex].textureCoord = Vector2( 0.0f, verticalTextureCoord );
      vertices[vertexIndex+1].textureCoord = Vector2( 1.0f, verticalTextureCoord );
    }

    // Flip the vertical texture coord for the UV values of the bottom points.
    verticalTextureCoord = 1.0f - verticalTextureCoord;

    // Bottom face, outward normals.
    for( int i = 0; i < 4; ++i, vertexIndex += 2 )
    {
      vertices[vertexIndex].position = positions[i + 4];
      vertices[vertexIndex].normal = normals[i + 1];

      if( i == 3 )
      {
        // End, so loop around.
        vertices[vertexIndex + 1].position = positions[4];
      }
      else
      {
        vertices[vertexIndex + 1].position = positions[i + 5];
      }
      vertices[vertexIndex + 1].normal = normals[i + 1];

      vertices[vertexIndex].textureCoord = Vector2( 0.0f, verticalTextureCoord );
      vertices[vertexIndex+1].textureCoord = Vector2( 1.0f, verticalTextureCoord );
    }

    // Bottom face, downward normals.
    for( int i = 0; i < 4; ++i, ++vertexIndex )
    {
      // Reverse positions for bottom face to keep triangles clockwise (for culling).
      vertices[vertexIndex].position = positions[ 7 - i ];
      vertices[vertexIndex].normal = normals[5];
      // The below logic forms the correct U/V pairs for a quad when "i" goes from 0 to 3.
      vertices[vertexIndex].textureCoord = Vector2( ( i == 1 || i == 2 ) ? 1.0f : 0.0f, ( i == 2 || i == 3 ) ? 1.0f : 0.0f );
    }

    // Create cube indices.
    int triangleIndex = 0u;     //Track progress through indices.
    indices.Resize( 3u * 12u ); // 3 points x 12 triangles.

    // Top face.
    indices[triangleIndex] =     0;
    indices[triangleIndex + 1] = 1;
    indices[triangleIndex + 2] = 2;
    indices[triangleIndex + 3] = 2;
    indices[triangleIndex + 4] = 3;
    indices[triangleIndex + 5] = 0;
    triangleIndex += 6;

    int topFaceStart = 4u;
    int bottomFaceStart = topFaceStart + 8u;

    // Side faces.
    for( int i = 0; i < 8; i += 2, triangleIndex += 6 )
    {
      indices[triangleIndex    ] = i + topFaceStart;
      indices[triangleIndex + 1] = i + bottomFaceStart + 1;
      indices[triangleIndex + 2] = i + topFaceStart + 1;
      indices[triangleIndex + 3] = i + topFaceStart;
      indices[triangleIndex + 4] = i + bottomFaceStart;
      indices[triangleIndex + 5] = i + bottomFaceStart + 1;
    }

    // Bottom face.
    indices[triangleIndex] =     20;
    indices[triangleIndex + 1] = 21;
    indices[triangleIndex + 2] = 22;
    indices[triangleIndex + 3] = 22;
    indices[triangleIndex + 4] = 23;
    indices[triangleIndex + 5] = 20;

    // Use the helper method to create the geometry object.
    return CreateTexturedGeometry( vertices, indices );
  }

private:

  Application&  mApplication;

  //The models displayed on screen, including information about rotation.
  Model mModels[NUM_MESHES];
  Actor mContainers[NUM_MESHES];

  //Acts as a global light source, which can be dragged around.
  Control mLightSource;

  //Used to detect panning to rotate the selected model.
  Vector2 mPanStart;
  Vector2 mRotationStart;

  int mModelIndex; //Index of model to load.
  int mShadingModeIndex; //Index of shader type to use.
  int mTag; //Identifies what kind of actor has been selected in OnTouch.
  int mSelectedModelIndex; //Index of model selected on screen.
  bool mPaused; //If true, all animations are paused and should stay so.
  bool mLightFixed; //If false, the light is in manual.
};

// Entry point for Linux & Tizen applications
//
int main( int argc, char **argv )
{
  Application application = Application::New( &argc, &argv );
  MeshVisualController test( application );
  application.MainLoop();
  return 0;
}
