// -*-c++-*-

/*
 *Copyright:

 Copyright (C) Hidehisa AKIYAMA

 This code is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3, or (at your option)
 any later version.

 This code is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this code; see the file COPYING.  If not, write to
 the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

 *EndCopyright:
 */

/////////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "bhv_basic_move.h"

#include "strategy.h"

#include "bhv_basic_tackle.h"
#include "my_Q.h"

#include <rcsc/action/basic_actions.h>
#include <rcsc/action/body_go_to_point.h>
#include <rcsc/action/body_intercept.h>
#include <rcsc/action/neck_turn_to_ball_or_scan.h>
#include <rcsc/action/neck_turn_to_low_conf_teammate.h>

#include <rcsc/player/player_agent.h>
#include <rcsc/player/debug_client.h>
#include <rcsc/player/intercept_table.h>

#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>

#include "neck_offensive_intercept_neck.h"

using namespace rcsc;


/*-------------------------------------------------------------------*/
/*!

 */
Q q;
int D_Step = 0;

bool
Bhv_BasicMove::execute2( PlayerAgent * agent )
{   
   //read();
    
    dlog.addText( Logger::TEAM,
                  __FILE__": Bhv_BasicMove" );

    //-----------------------------------------------
    // tackle
    if ( Bhv_BasicTackle( 0.8, 80.0 ).execute( agent ) )
    {
        return true;
    }
    
    ++D_Step;

    const WorldModel & wm = agent->world();


    /*--------------------------------------------------------*/
    // chase ball
    const int self_min = wm.interceptTable()->selfReachCycle();
    const int mate_min = wm.interceptTable()->teammateReachCycle();
    const int opp_min = wm.interceptTable()->opponentReachCycle();

    if ( ! wm.existKickableTeammate()
         && ( self_min <= 10
              || ( self_min <= mate_min
                   && self_min < opp_min + 3 )
              )
         )  
    {
        dlog.addText( Logger::TEAM,
                      __FILE__": intercept" );
        Body_Intercept().execute( agent );
        agent->setNeckAction( new Neck_OffensiveInterceptNeck() );
        return true;
    }
   
   if(D_Step == 1) {
 
    q.setQ();
   }


    std::string myMapKey;
    std::string myMapVal;
    std::vector<double> oldState;
    std::vector<double> newState;
    int action;
    int reward;
    q.NewPlayer();
    q.NewAgent(3);
    q.myPlayer.State[0] = wm.self().distFromBall();
    q.myPlayer.State[1] = wm.ball().pos().y - wm.self().pos().y;
    // myPlayer.State[0] = wm.self().pos().x;
    // myPlayer.State[1] = wm.self().pos().y;
    oldState = q.getState();
    
    action = q.ChooseAction(oldState);

    std::cout << "D_step:" << D_Step << std::endl;
    if (D_Step >= 2){

        if ( wm.self().kicked() ){
            reward = 10;
        } else {
            reward = -1;
        }

        Vector2D self_next = wm.self().pos() + wm.self().vel();
        Vector2D ball_next = wm.ball().pos() + wm.ball().vel();
        q.myPlayer.State[0] = self_next.dist2(ball_next);
        q.myPlayer.State[1] = self_next.y-ball_next.y;
        newState = q.getState();
        std::cout << oldState[0] << "," << oldState[1] << std::endl;
        q.UpdataQ(oldState, action, reward, newState);
        for(int i =0 ; i<4 ;i++){
            std::cout << q.myAgent.Qtable[oldState][i] << ",";
        }
        std::cout << std::endl;

        myMapKey = q.convertDoubleToString(oldState[0]) + "," + q.convertDoubleToString(oldState[1]);
        myMapVal = q.convertDoubleToString(q.myAgent.Qtable[oldState][0]) + "," +
                q.convertDoubleToString(q.myAgent.Qtable[oldState][1]) + "," +
                q.convertDoubleToString(q.myAgent.Qtable[oldState][2]) + "," +
                q.convertDoubleToString(q.myAgent.Qtable[oldState][3]);

       if (q.myMap.count(myMapKey) > 0 ){
           q.myMap[myMapKey] = myMapVal;
       } else {
            q.myMap.insert(make_pair(myMapKey,myMapVal));
       }

       if(D_Step == 900) {
 
            q.save();
        }
        
       
    }
     
    
    //const Vector2D target_point = Strategy::i().getPosition( wm.self().unum() );
    
    //Vector2D target_point;
    const double dash_power = Strategy::get_normal_dash_power( wm );
    
    Vector2D mypoint;
    mypoint.x = wm.self().pos().x + q.myPlayer.Actions[action][0];
    mypoint.y =  wm.self().pos().y + q.myPlayer.Actions[action][1];
    if (wm.self().distFromBall() > 60 ){
        mypoint.x = wm.self().pos().x;
    }
    const Vector2D target_point = mypoint;

    double dist_thr = wm.ball().distFromSelf() * 0.1;
    if ( dist_thr < 1.0 ) dist_thr = 1.0;

    dlog.addText( Logger::TEAM,
                  __FILE__": Bhv_BasicMove target=(%.1f %.1f) dist_thr=%.2f",
                  target_point.x, target_point.y,
                  dist_thr );

    agent->debugClient().addMessage( "BasicMove%.0f", dash_power );
    agent->debugClient().setTarget( target_point );
    agent->debugClient().addCircle( target_point, dist_thr );

    if ( ! Body_GoToPoint( target_point, dist_thr, dash_power
                           ).execute( agent ) )
    {
        Body_TurnToBall().execute( agent );
    }

    if ( wm.existKickableOpponent()
         && wm.ball().distFromSelf() < 18.0 )
    {
        agent->setNeckAction( new Neck_TurnToBall() );
    }
    else
    {
        agent->setNeckAction( new Neck_TurnToBallOrScan() );
    }

    return true;
}






bool
Bhv_BasicMove::execute( PlayerAgent * agent )
{
    dlog.addText( Logger::TEAM,
                  __FILE__": Bhv_BasicMove" );

    //-----------------------------------------------
    // tackle
    if ( Bhv_BasicTackle( 0.8, 80.0 ).execute( agent ) )
    {
        return true;
    }

    const WorldModel & wm = agent->world();
    /*--------------------------------------------------------*/
    // chase ball
    const int self_min = wm.interceptTable()->selfReachCycle();
    const int mate_min = wm.interceptTable()->teammateReachCycle();
    const int opp_min = wm.interceptTable()->opponentReachCycle();

    if ( ! wm.existKickableTeammate()
         && ( self_min <= 3
              || ( self_min <= mate_min
                   && self_min < opp_min + 3 )
              )
         )
    {
        dlog.addText( Logger::TEAM,
                      __FILE__": intercept" );
        Body_Intercept().execute( agent );
        agent->setNeckAction( new Neck_OffensiveInterceptNeck() );

        return true;
    }

    const Vector2D target_point = Strategy::i().getPosition( wm.self().unum() );
    const double dash_power = Strategy::get_normal_dash_power( wm );

    double dist_thr = wm.ball().distFromSelf() * 0.1;
    if ( dist_thr < 1.0 ) dist_thr = 1.0;

    dlog.addText( Logger::TEAM,
                  __FILE__": Bhv_BasicMove target=(%.1f %.1f) dist_thr=%.2f",
                  target_point.x, target_point.y,
                  dist_thr );

    agent->debugClient().addMessage( "BasicMove%.0f", dash_power );
    agent->debugClient().setTarget( target_point );
    agent->debugClient().addCircle( target_point, dist_thr );

    if ( ! Body_GoToPoint( target_point, dist_thr, dash_power
                           ).execute( agent ) )
    {
        Body_TurnToBall().execute( agent );
    }

    if ( wm.existKickableOpponent()
         && wm.ball().distFromSelf() < 18.0 )
    {
        agent->setNeckAction( new Neck_TurnToBall() );
    }
    else
    {
        agent->setNeckAction( new Neck_TurnToBallOrScan() );
    }

    return true;
}
