/*
      This file is part of Smoothie (http://smoothieware.org/). The motion control part is heavily based on Grbl (https://github.com/simen/grbl).
      Smoothie is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
      Smoothie is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
      You should have received a copy of the GNU General Public License along with Smoothie. If not, see <http://www.gnu.org/licenses/>.
*/

#include "libs/Module.h"
#include "libs/Kernel.h"
#include <math.h>
using namespace std;
#include <vector>
#include "ToolManager.h"
#include "PublicDataRequest.h"
#include "ToolManagerPublicAccess.h"
#include "Config.h"
#include "Robot.h"
#include "ConfigValue.h"
#include "Conveyor.h"
#include "checksumm.h"
#include "PublicData.h"
#include "Gcode.h"

#include "libs/SerialMessage.h"
#include "libs/StreamOutput.h"
#include "FileStream.h"

#include "modules/robot/RobotPublicAccess.h"

#define return_error_on_unhandled_gcode_checksum    CHECKSUM("return_error_on_unhandled_gcode")

#define X_AXIS      0
#define Y_AXIS      1
#define Z_AXIS      2

ToolManager::ToolManager(){
    active_tool = 0;
    current_tool_name = CHECKSUM("hotend");
}

void ToolManager::on_module_loaded(){
    this->on_config_reload(this);

    this->register_for_event(ON_CONFIG_RELOAD);
    this->register_for_event(ON_GCODE_RECEIVED);
    this->register_for_event(ON_GET_PUBLIC_DATA);
    this->register_for_event(ON_SET_PUBLIC_DATA);
}

void ToolManager::on_config_reload(void *argument){
    return_error_on_unhandled_gcode = THEKERNEL->config->value( return_error_on_unhandled_gcode_checksum )->by_default(false)->as_bool();
}

void ToolManager::on_gcode_received(void *argument){
    Gcode *gcode = static_cast<Gcode*>(argument);

    if( gcode->has_letter('T') ){
        int new_tool = gcode->get_value('T');
        bool make_move = false;
        if ( gcode->has_letter('F') ){
            make_move = true;
        }
        gcode->mark_as_taken();
        if(new_tool >= (int)this->tools.size() || new_tool < 0){
            // invalid tool
            if( return_error_on_unhandled_gcode ) {
                char buf[32]; // should be big enough for any status
                int n= snprintf(buf, sizeof(buf), "T%d invalid tool ", new_tool);
                gcode->txt_after_ok.append(buf, n);
            }
        } else {
        
            //send new_tool_offsets to robot
            float new_pos[3];
            float *active_tool_offset = tools[this->active_tool]->get_offset();
            float *new_tool_offset = tools[new_tool]->get_offset();
            THEKERNEL->robot->setToolOffset(new_tool_offset[0], new_tool_offset[1], new_tool_offset[2]);
        
            if(new_tool != this->active_tool){
                void *returned_data;
                THEKERNEL->conveyor->wait_for_empty_queue();
                bool ok = THEKERNEL->public_data->get_value( robot_checksum, current_position_checksum, &returned_data );
                if(ok){
                    // save current position to return to after applying extruder offset
                    float *pos = static_cast<float *>(returned_data);
                    float current_pos[3];
                    for(int i=0;i<3;i++){
                        current_pos[i] = pos[i];
                    }
                    // update virtual tool position to the offset of the new tool and select it as active

                    for(int i=0;i<3;i++){
                        new_pos[i] = current_pos[i] - active_tool_offset[i] + new_tool_offset[i];
                    }

                    this->tools[active_tool]->disable();
                    this->active_tool = new_tool;
                    this->current_tool_name = this->tools[active_tool]->get_name();
                    this->tools[active_tool]->enable();

                    if(make_move){
                        //move to old position
                        char buf[32];
                        string s = buf;
                        Gcode *g = new Gcode(s, gcode->stream);
                        snprintf(buf, 31, "G0 X%g Y%g Z%g", current_pos[X_AXIS], current_pos[Y_AXIS], current_pos[Z_AXIS]);
                        s = buf;
                        g = new Gcode(s, gcode->stream);
                        THEKERNEL->call_event(ON_GCODE_RECEIVED, g);
                        delete g;
                    }
                }
            }
        }
    }
}

void ToolManager::on_get_public_data(void* argument){
    PublicDataRequest* pdr = static_cast<PublicDataRequest*>(argument);

    if(!pdr->starts_with(tool_manager_checksum)) return;

    // ok this is targeted at us, so send back the requested data
    // this must be static as it will be accessed long after we have returned
    static struct pad_toolmanager tool_return;
    tool_return.current_tool_name= this->current_tool_name;

    pdr->set_data_ptr(&tool_return);
    pdr->set_taken();
}

void ToolManager::on_set_public_data(void* argument){
    PublicDataRequest* pdr = static_cast<PublicDataRequest*>(argument);

    if(!pdr->starts_with(tool_manager_checksum)) return;

    // ok this is targeted at us, so change tools
    uint16_t tool_name= *static_cast<float*>(pdr->get_data_ptr());
    // TODO: fire a tool change gcode
    pdr->set_taken();
}

// Add a tool to the tool list
void ToolManager::add_tool(Tool* tool_to_add){
    if(this->tools.size() == 0){
        tool_to_add->enable();
        this->current_tool_name = tool_to_add->get_name();
    } else {
        tool_to_add->disable();
    }
    this->tools.push_back( tool_to_add );
}


