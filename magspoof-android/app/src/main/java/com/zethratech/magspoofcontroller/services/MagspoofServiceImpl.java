/*
 * Magspoof Controller - An app to control a Magspoof
 *     Copyright (C) 2016  Ben Goldberg
 *
 *     This program is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 *
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

package com.zethratech.magspoofcontroller.services;

import android.app.Activity;
import android.util.Log;

import org.shokai.firmata.ArduinoFirmata;
import org.shokai.firmata.ArduinoFirmataDataHandler;
import org.shokai.firmata.ArduinoFirmataEventHandler;

import java.io.IOException;

public class MagspoofServiceImpl implements MagspoofService{

    private String TAG = "MagspoofController - MagspoofServiceImpl";
    private ArduinoFirmata arduino = null;

    public void init(final Activity activity) {
        arduino =  new ArduinoFirmata(activity);
        try{
            arduino.connect();
            Log.v(TAG, "Board Version : "+arduino.getBoardVersion());
        }
        catch(IOException | InterruptedException e){
            e.printStackTrace();
        }
        arduino.setEventHandler(new ArduinoFirmataEventHandler(){
            public void onError(String errorMessage){
                Log.e(TAG, errorMessage);
            }
            public void onClose(){
                Log.v(TAG, "arduino closed");
            }
        });
        arduino.setDataHandler(new ArduinoFirmataDataHandler(){
            public void onSysex(byte command, byte[] data){
                String dataStr = "";
                for (byte b : data) {
                    dataStr += Integer.valueOf(b).toString();
                    dataStr += ",";
                }
                Log.v(TAG, "sysex command : " + Integer.valueOf(command).toString() +
                        "\nsysex data    : " + "["+dataStr+"]");
            }
        });
    }

    public void sendID(String id) {
        Log.v(TAG, "Sending id: " + id);
        arduino.sysex((byte)0x71, id.getBytes());
    }

    public void stop() {
        if(arduino == null)
            return;
        arduino.close();
    }

    public String getFirmataVersion() {
        return ArduinoFirmata.VERSION;
    }

}
