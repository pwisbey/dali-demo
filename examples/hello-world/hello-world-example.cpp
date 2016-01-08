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
#include <stdio.h>

using namespace Dali;
using namespace Dali::Toolkit;

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
    stage.SetBackgroundColor( Color::WHITE );

    Control actor1 = Toolkit::Control::New();
    actor1.SetBackgroundColor( Vector4( 1.0f, 0.0f, 0.0f, 1.0f ) );
    actor1.SetAnchorPoint( AnchorPoint::TOP_LEFT );
    actor1.SetParentOrigin( ParentOrigin::TOP_LEFT );
    actor1.SetSize( 100.0f, 100.0f );
    actor1.SetPosition( 50.0f, 50.0f );
    actor1.SetName( "Actor1" );
    actor1.TouchedSignal().Connect( this, &HelloWorldController::OnTouch );
    stage.Add( actor1 );

    Control actor2 = Toolkit::Control::New();
    actor2.SetBackgroundColor( Vector4( 0.0f, 1.0f, 0.0f, 1.0f ) );
    actor2.SetAnchorPoint( AnchorPoint::TOP_LEFT );
    actor2.SetParentOrigin( ParentOrigin::TOP_LEFT );
    actor2.SetSize( 100.0f, 100.0f );
    actor2.SetPosition( 200.0f, 50.0f );
    actor2.SetName( "Actor2" );
    actor2.TouchedSignal().Connect( this, &HelloWorldController::OnTouch );
    stage.Add( actor2 );

    Control actor3 = Toolkit::Control::New();
    actor3.SetBackgroundColor( Vector4( 0.0f, 0.0f, 1.0f, 1.0f ) );
    actor3.SetAnchorPoint( AnchorPoint::TOP_LEFT );
    actor3.SetParentOrigin( ParentOrigin::TOP_LEFT );
    actor3.SetSize( 100.0f, 100.0f );
    actor3.SetPosition( 350.0f, 50.0f );
    actor3.SetName( "Actor3" );
    actor3.TouchedSignal().Connect( this, &HelloWorldController::OnTouch );
    stage.Add( actor3 );

    Control actor4 = Toolkit::Control::New();
    actor4.SetBackgroundColor( Vector4( 1.0f, 1.0f, 0.0f, 1.0f ) );
    actor4.SetAnchorPoint( AnchorPoint::TOP_LEFT );
    actor4.SetParentOrigin( ParentOrigin::TOP_LEFT );
    actor4.SetSize( 100.0f, 100.0f );
    actor4.SetPosition( 50.0f, 200.0f );
    actor4.SetName( "Actor4" );
    actor4.TouchedSignal().Connect( this, &HelloWorldController::OnTouch );
    stage.Add( actor4 );

    Control actor5 = Toolkit::Control::New();
    actor5.SetBackgroundColor( Vector4( 0.0f, 1.0f, 1.0f, 1.0f ) );
    actor5.SetAnchorPoint( AnchorPoint::TOP_LEFT );
    actor5.SetParentOrigin( ParentOrigin::TOP_LEFT );
    actor5.SetSize( 100.0f, 100.0f );
    actor5.SetPosition( 200.0f, 200.0f );
    actor5.SetName( "Actor5" );
    actor5.TouchedSignal().Connect( this, &HelloWorldController::OnTouch );
    stage.Add( actor5 );

    Control actor6 = Toolkit::Control::New();
    actor6.SetBackgroundColor( Vector4( 1.0f, 0.0f, 1.0f, 1.0f ) );
    actor6.SetAnchorPoint( AnchorPoint::TOP_LEFT );
    actor6.SetParentOrigin( ParentOrigin::TOP_LEFT );
    actor6.SetSize( 100.0f, 100.0f );
    actor6.SetPosition( 350.0f, 200.0f );
    actor6.SetName( "Actor6" );
    actor6.TouchedSignal().Connect( this, &HelloWorldController::OnTouch );
    stage.Add( actor6 );
  }

  bool OnTouch( Actor actor, const TouchEvent& touch )
  {
    const TouchPoint& point = touch.GetPoint(0);

    switch( point.state )
    {
      case TouchPoint::Down:
      {
        printf( "TouchPoint::Down: %s\n", actor.GetName().c_str() );
        actor.SetDrawMode( DrawMode::OVERLAY_2D );
        mTouch = point.screen;
        break;
      }
      case TouchPoint::Motion:
      {
        actor.TranslateBy( Vector3( point.screen.x - mTouch.x, point.screen.y - mTouch.y, 0.0f ) );
        mTouch = point.screen;
        break;
      }
      case TouchPoint::Up:
      {
        printf( "TouchPoint::Up: %s\n", actor.GetName().c_str() );
        actor.SetDrawMode( DrawMode::NORMAL );
        break;
      }
      case TouchPoint::Interrupted:
      {
        printf( "TouchPoint::Interrupted: %s\n", actor.GetName().c_str() );
        actor.SetDrawMode( DrawMode::NORMAL );
        break;
      }
      default:
      {
        break;
      }
    }

    return true;
  }

private:
  Application&  mApplication;
  Vector2 mTouch;
};

void RunTest( Application& application )
{
  HelloWorldController test( application );

  application.MainLoop();
}

// Entry point for Linux & Tizen applications
//
int main( int argc, char **argv )
{
  Application application = Application::New( &argc, &argv );

  RunTest( application );

  return 0;
}
