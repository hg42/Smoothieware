/*
      This file is part of Smoothie (http://smoothieware.org/). The motion control part is heavily based on Grbl (https://github.com/simen/grbl).
      Smoothie is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
      Smoothie is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
      You should have received a copy of the GNU General Public License along with Smoothie. If not, see <http://www.gnu.org/licenses/>.
*/

#include "libs/Module.h"
#include "libs/Kernel.h"

Module::Module(){}
Module::~Module(){}

void Module::register_for_event_(_EVENT_ENUM event_id, ModuleCallback event_handler){
    // Events are the basic building blocks of Smoothie. They register for events, and then do stuff when those events are called.
    // You add things to Smoothie by making a new class that inherits the Module class. See http://smoothieware.org/moduleexample for a crude introduction
    THEKERNEL->register_for_event_(event_id, event_handler);
}
