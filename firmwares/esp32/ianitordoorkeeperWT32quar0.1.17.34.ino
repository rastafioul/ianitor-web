
  /*https://github.com/Links2004/arduinoWebSockets/
 * WebSocketClientSocketIOack.ino
 *
 *  Created on: 20.07.2019
 *
 */
 /*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-save-data-permanently-preferences/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/
/*********
  Rui Santos
  Complete instructions at https://RandomNerdTutorials.com/esp32-wi-fi-manager-asyncwebserver/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*********/


 // Verifier: https://www.esp32.com/viewtopic.php?t=19135

//#include <Arduino.h>

#define USE_SERIAL Serial

//Serial.println("************************************************************************************************************");
//Serial.println("***********                                                                                 ****************")
//Serial.printf("ESP32 Chip model = %s Rev %d\n", ESP.getChipModel(), ESP.getChipRevision());
//Serial.print("\n Starting board " + String(ARDUINO_BOARD));
//Serial.println(" with " + String(SHIELD_TYPE));

String version = "0.1.7.17.34.wt32"; 

//
//#define DEBUG_ETHERNET_WEBSERVER_PORT       Serial
//// Debug Level from 0 to 4
#define _ETHERNET_WEBSERVER_LOGLEVEL_       3

#include <WebServer_WT32_ETH01.h>

//#include "FS.h"                   //this needs to be first, or it all crashes and burns...
//#include "LittleFS.h"
//#include "stdlib.h"

#include <Preferences.h>          //finalement, ca a l'air d'etre plus facile a utiliser

#include <WiFi.h>
#include <WiFiMulti.h>            //wifimulti_generic depuisrestartAfterWifi bibliotheque
#include <WiFiClientSecure.h>     //depuis bibliotheque
#include <HTTPClient.h>           //depuis bibliotheque
//#include <esp_wifi.h>           // for test: to get ssid when not connected
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>             //pour otaser
#include <AsyncElegantOTA.h>      //pour ota
//#include <Update.h>               //pour ota


#include <ArduinoJson.h>

#include <WebSocketsClient.h> //https://github.com/Links2004/arduinoWebSockets/ semble se charger tout seul
#include <SocketIOclient.h>   //https://github.com/Links2004/arduinoWebSockets/ semble se charger tout seul

//Est-ce vraiment utile?
#include <ESPDateTime.h>      //https://github.com/mcxiaoke/ESPDateTime


WiFiMulti WiFiMulti;
bool wifimulti = false;
bool isWt32 = false;
bool forceWt32 = false;
bool useEth = false;
int phy_addr_default = -1;
int myPhy_addr = -1;
SocketIOclient socketIO;

TaskHandle_t TaskNotifyConnected;

// #define USE_SERIAL Serial

Preferences preferences;
String prefNameSpace = "ianitorConf";

//preferences.begin(prefNameSpace.c_str(), false);
//  isWt32 = preferences.getBool("isWt32", false);
//preferences.end();
//if(isWt32)
//{
//   #include <WebServer_WT32_ETH01.h>
//}
//else
//{
//  forceWt32 = false;
//  useEth = false;
//}

// Timer variables
unsigned long previousMillis = 0;
const long interval = 30000;  // interval to wait for Wi-Fi connection (milliseconds)

unsigned long messageTimestamp = 0;
uint64_t now = millis();
const long ntpDelay = 5000; //par defaut cela devrait etre 10 sec
const long intervalSocketCheck = 5000;
unsigned long previousSocketCheck = 0;
int iteSocketCheck = 0;
unsigned int maxIteSockDownBeforeAP = 5;

             

//fait un check d'action effectuer avant de faire un rechargemetn de l'esp:
//cas ou wifi down, du coup la socket deconnectee, elle remet le wifi mais personne ne vient
// donc tentative de reconnection au wifi.
const long intervalNoWifiActionCheck = 5000;
unsigned long previousNoWifiActionCheck = 0;
int iteNoWifiActionCheck = 0;
unsigned int maxIteNoWifiActionBeforeReboot = 10;


bool inverted = true;

String trueFirmwareUpdate = "false";
int updateFirmware_idComment = 0;
String updateFirmware_idSocketCaller = "idSocketCaller";
String updateFirmware_idCaller = "idCaller";
String updateFirmware_type = "typeToFind";
String updateFirmware_ents = "ents";
String updateFirmware_oldVers = "";



/*******************************************************************************************/
/************************ pages web ********************************************************/

String wifimanagerHTML = R"(
<!DOCTYPE html>
<html>
<head>
  <title>Ianitor DoorKeeper Wi-Fi Manager</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta charset="UTF-8">
  <link rel="icon" href="data:,">
  <!--link rel="stylesheet" type="text/css" href="style.css"-->
  <style>

html {
    font-family: Arial, Helvetica, sans-serif; 
    display: inline-block; 
    text-align: center;
  }
  
  h1 {
    font-size: 1.8rem; 
    color: white;
  }

  h2 {
    background-color: lightgray;
  }
  
  p { 
    font-size: 1.4rem;
  }

  .onlywifi{
    background-color: blue;
   }
  
  .topnav { 
    overflow: hidden; 
    background-color: #0A1128;
  }
  
  body {  
    margin: 0;
  }
  
  .content { 
    padding: 5%;
  }
  
  .card-grid { 
    max-width: 800px; 
    margin: 0 auto; 
    display: grid; 
    grid-gap: 2rem; 
    grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
  }
  
  .card { 
    background-color: white; 
    box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5);
  }
  
  .card-title { 
    font-size: 1.2rem;
    font-weight: bold;
    color: #034078
  }
  
  input[type=submit] {
    border: none;
    color: #FEFCFB;
    background-color: #034078;
    padding: 15px 15px;
    text-align: center;
    text-decoration: none;
    display: inline-block;
    font-size: 16px;
    width: 100px;
    margin-right: 10px;
    border-radius: 4px;
    transition-duration: 0.4s;
    }
  
  input[type=submit]:hover {
    background-color: #1282A2;
  }

  input[type=submit]:disabled {
    opacity: 50%;
  }
  
  input[type=text], input[type=number], select {
    width: 50%;
    padding: 12px 20px;
    margin: 18px;
    display: inline-block;
    border: 1px solid #ccc;
    border-radius: 4px;
    box-sizing: border-box;
  }
  
  label {
    font-size: 1.2rem; 
  }
  .value{
    font-size: 1.2rem;
    color: #1282A2;  
  }
  .state {
    font-size: 1.2rem;
    color: #1282A2;
  }
  button {
    border: none;
    color: #FEFCFB;
    padding: 15px 32px;
    text-align: center;
    font-size: 16px;
    width: 100px;
    border-radius: 4px;
    transition-duration: 0.4s;
  }
  .button-on {
    background-color: #034078;
  }
  .button-on:hover {
    background-color: #1282A2;
  }
  .button-off {
    background-color: #858585;
  }
  .button-off:hover {
    background-color: #252524;
  } 





  
     /* The Modal (background) */
.modal {
  display: none; /* Hidden by default */
  position: fixed; /* Stay in place */
  z-index: 1; /* Sit on top */
  left: 0;
  top: 0;
  width: 100%; /* Full width */
  height: 100%; /* Full height */
  overflow: auto; /* Enable scroll if needed */
  background-color: rgb(0,0,0); /* Fallback color */
  background-color: rgba(0,0,0,0.4); /* Black w/ opacity */
}

/* Modal Content/Box */
.modal-content {
  background-color: #fefefe;
  margin: 15% auto; /* 15% from the top and centered */
  padding: 20px;
  border: 1px solid #888;
  width: 80%; /* Could be more or less, depending on screen size */
}

/* The Close Button */
.close {
  color: #aaa;
  float: right;
  font-size: 28px;
  font-weight: bold;
}

.close:hover,
.close:focus {
  color: black;
  text-decoration: none;
  cursor: pointer;
} 
  </style>

  <script type="application/javascript" src="getwifis.js"></script>
  <script type="application/javascript" src="getvariables.js"></script>
  <script>
     var modal;
     var modalPwd;
     var span;
     let editingPwd = "";
     let validEditingPwd = false;
    function edit()
    {
      modal.style.display = "block";
    }
    function showModal(_modal)
    {
      document.getElementById(_modal).style.display = "block";
    }
    function checkPassword()
    {
      document.getElementById("resultCheck" ).innerHTML="checking password...";

      var onlineURL = "/checkPassword"
      let xhr = new XMLHttpRequest();
        xhr.open("GET", onlineURL, true);
        xhr.setRequestHeader("Authorization", "Bearer " + btoa(document.getElementById("editingPwd" ).value)) ;
        xhr.onreadystatechange = function() {
            if (xhr.readyState === 4) {
                if (xhr.status === 200) {
                    var jsonRules = JSON.parse(xhr.responseText).result;
                    var db = "onlineRules";
                    console.log("checkPassword: "+xhr.responseText);
                    if ( jsonRules==true || jsonRules=="true" )
                    {
                      freeEdition(true);
                      editingPwd = btoa(document.getElementById("editingPwd" ).value);
                      validEditingPwd = true;
                      document.getElementById("tokenField" ).value = editingPwd;
                    }
                    else{
                      freeEdition(false);
                    }
                } else {
                    document.getElementById("resultCheck" ).innerHTML="error connexion: "+xhr.status+"<br>"+xhr.responseText;
                }
            }
        }
        xhr.send();
    }
    function updatePassword()
    {
      document.getElementById("resultCheck" ).innerHTML="updating password...";
      document.getElementById("resultCheckPwd" ).innerHTML="updating password...";


      var onlineURL = "/setpassword"
      let xhr = new XMLHttpRequest();
        xhr.open("POST", onlineURL, true);
        xhr.setRequestHeader("Authorization", "Bearer " + editingPwd) ;
        xhr.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
        var params = "newPwd="+btoa(document.getElementById("updatePwd" ).value);
        console.log("sending updatePassword: "+params);
        console.log("sending updatePassword xhr.responseText: "+xhr.responseText);
        xhr.onreadystatechange = function() {
            if (xhr.readyState === 4) {
                if (xhr.status === 200) {
                    var jsonRules = JSON.parse(xhr.responseText).result;
                    var db = "onlineRules";
                    console.log("updatePassword: "+xhr.responseText);
                    if ( jsonRules==true || jsonRules=="true" )
                    {
                      document.getElementById("resultCheck" ).innerHTML="";

                      document.getElementById("resultCheckPwd" ).innerHTML = `<strong>password update done, remember it well!</strong><br><button onclick='closeModal("myModalpwd" );' style='color:black !important'>close</button>`;
                      editingPwd =  btoa(document.getElementById("updatePwd" ).value);
                      document.getElementById("tokenField" ).value = editingPwd;
                      
                      //closeModal("myModalpwd");
                    }
                    else{
                      document.getElementById("resultCheckPwd" ).innerHTML = "<strong style=\"color:red\">password update not done<strong>";
                    }
                } else {
                    document.getElementById("resultCheckPwd" ).innerHTML="error connexion: "+xhr.status+"<br>"+xhr.responseText;
                }
            }else
            {
             console.log("readyState:"+xhr.readyState+"" );
             console.log("status code:"+xhr.status+"" ); 
             console.log("reponse:"+xhr.responseText+"" ); 
            }
        }
        xhr.send(params);
    }
    function freeEdition(_bool)
    {
      if(_bool)
      {
          document.getElementById("ssid" ).removeAttribute("disabled" );
          document.getElementById("wifipass" ).removeAttribute("disabled" );
          document.getElementById("hostname" ).removeAttribute("disabled" );
          document.getElementById("login" ).removeAttribute("disabled" );
          document.getElementById("pwd" ).removeAttribute("disabled" );
          document.getElementById("submitwifi" ).removeAttribute("disabled" );
          document.getElementById("submitCredentials" ).removeAttribute("disabled" );
          document.getElementById("submitUpdate" ).removeAttribute("disabled" );
          document.getElementById("submitStaticIp" ).removeAttribute("disabled" );
          document.getElementById("showUpdatePwdBtn" ).removeAttribute("disabled" );

          
          document.getElementById("wt32Conf").removeAttribute("disabled");
          document.getElementById("staticIpWifiConf").removeAttribute("disabled");
          document.getElementById("staticIpEthConf").removeAttribute("disabled");
          

          document.getElementById("useEth_True" ).removeAttribute("disabled" );
          document.getElementById("useEth_False" ).removeAttribute("disabled" );
          
          document.getElementById("StaticIpChoice1" ).removeAttribute("disabled" );
          document.getElementById("StaticIpChoice2" ).removeAttribute("disabled" );

//          if(document.getElementById("useEth_True" ).checked)
//          {
//            document.getElementById("StaticIpChoice1_eth" ).removeAttribute("disabled" );
//            document.getElementById("StaticIpChoice2_eth" ).removeAttribute("disabled" );
//          }
          if(document.getElementById("StaticIpChoice1").checked )
            allowStaticIp(true);
            
          if(document.getElementById("useEth_True" ).checked)
          {
            document.getElementById("StaticIpChoice1_eth" ).removeAttribute("disabled" );
            document.getElementById("StaticIpChoice2_eth" ).removeAttribute("disabled" );
            if(document.getElementById("StaticIpChoice1_eth").checked )
              allowStaticIp_eth(true);
          }
            
          if(document.getElementById("isWt32").checked )
            allowWt32edition(true);
          else 
            allowWt32edition(false);
            
          document.getElementById("myPhy_addr").removeAttribute("disabled" );
          
          modal.style.display = "none";
      }
      else{
        document.getElementById("resultCheck").innerHTML="bad password";
      }
    }
    function allowWt32edition(_bool)
    {
      if(_bool)
      {
        document.getElementById("useEth_True" ).removeAttribute("disabled" );
        document.getElementById("mask_eth" ).removeAttribute("disabled" );
        document.getElementById("myPhy_addr" ).removeAttribute("disabled" );
        document.getElementById("StaticIpChoice1_eth" ).removeAttribute("disabled" );
        document.getElementById("StaticIpChoice2_eth" ).removeAttribute("disabled" );
      }
      else
      {
        document.getElementById("useEth_True" ).setAttribute("disabled", true );
        document.getElementById("useEth_False" ).setAttribute("disabled", true );
        document.getElementById("myPhy_addr" ).setAttribute("disabled", true );
        document.getElementById("StaticIpChoice1_eth" ).setAttribute("disabled", true );
        document.getElementById("StaticIpChoice2_eth" ).setAttribute("disabled", true );
      }
    }
    
    function allowStaticIp_eth(_bool)
    {
      if(_bool)
      {
        document.getElementById("ip_eth" ).removeAttribute("disabled" );
        document.getElementById("mask_eth" ).removeAttribute("disabled" );
        document.getElementById("gateway_eth" ).removeAttribute("disabled" );
        document.getElementById("dns_eth" ).removeAttribute("disabled" );
      }
      else
      {
        document.getElementById("ip_eth" ).setAttribute("disabled", true );
        document.getElementById("mask_eth" ).setAttribute("disabled", true );
        document.getElementById("gateway_eth" ).setAttribute("disabled", true );
        document.getElementById("dns_eth" ).setAttribute("disabled", true );
      }
    }
    
    function allowStaticIp(_bool)
    {
      if(_bool)
      {
        document.getElementById("ip" ).removeAttribute("disabled" );
        document.getElementById("mask" ).removeAttribute("disabled" );
        document.getElementById("gateway" ).removeAttribute("disabled" );
        document.getElementById("dns" ).removeAttribute("disabled" );
      }
      else
      {
        document.getElementById("ip" ).setAttribute("disabled", true );
        document.getElementById("mask" ).setAttribute("disabled", true );
        document.getElementById("gateway" ).setAttribute("disabled", true );
        document.getElementById("dns" ).setAttribute("disabled", true );
      }
    }

    function setSsdiText(_ssid)
    {
      document.getElementById("ssid" ).value = _ssid;
    }
    function closeModal(_modal)
    {
      document.getElementById(_modal).style.display = "none";
    }



  function updateStaticIp()
    {
      //document.getElementById("resultCheck" ).innerHTML="updating staticIp conf...";
      //document.getElementById("resultCheckPwd" ).innerHTML="updating password...";


      var onlineURL = "/updateStaticIp"
      let xhr = new XMLHttpRequest();
        xhr.open("POST", onlineURL, true);
        xhr.setRequestHeader("Authorization", "Bearer " + editingPwd) ;
        xhr.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
        var params = "";
        if(document.getElementById("useStaticIp").checked == true || document.getElementById("useStaticIp").value == "true" || document.getElementById("useStaticIp").value == 1 || document.getElementById("useStaticIp_eth").checked == true || document.getElementById("useStaticIp_eth").value == "true"|| document.getElementById("useStaticIp_eth").value == 1)
        {
          params = "useStaticIp="+(document.getElementById("useStaticIp").checked).toString()
                      +"&ip="+document.getElementById("ip").value
                      +"&mask="+document.getElementById("mask").value
                      +"&gateway="+document.getElementById("gateway").value
                      +"&dns="+document.getElementById("dns").value
                   +"useStaticIp_eth="+(document.getElementById("useStaticIp_eth").checked).toString()
                      +"&ip_eth="+document.getElementById("ip_eth").value
                      +"&mask_eth="+document.getElementById("mask_eth").value
                      +"&gateway_eth="+document.getElementById("gateway_eth").value
                      +"&dns_eth="+document.getElementById("dns_eth").value;
        }else
        {
           params = "useStaticIp="+(document.getElementById("useStaticIp").checked).toString()
        }
        console.log("sending updateStaticIp: "+params);
        console.log("sending updateStaticIp xhr.responseText: "+xhr.responseText);
        xhr.onreadystatechange = function() {
            if (xhr.readyState === 4) {
                if (xhr.status === 200) {
                    var jsonRules = JSON.parse(xhr.responseText).result;
                    var db = "onlineRules";
                    console.log("updateStaticIp: "+xhr.responseText);
                    if ( jsonRules==true || jsonRules=="true" )
                    {
                      document.getElementById("resultStaticIP" ).innerHTML="";

                      document.getElementById("resultStaticIP" ).innerHTML = `<strong>Static Ip adress configuration update done</strong><br><button onclick='closeModal("myModalpwd" );' style='color:black !important'>close</button>`;
                      
                      //closeModal("myModalpwd");
                    }
                    else{
                      document.getElementById("resultStaticIP" ).innerHTML = "<strong style=\"color:red\">Static Ip adress configuration not done<strong>";
                    }
                } else {
                    document.getElementById("resultStaticIP" ).innerHTML="error connexion: "+xhr.status+"<br>"+xhr.responseText;
                }
            }else
            {
             console.log("updateStaticIp - readyState:"+xhr.readyState+"" );
             console.log("updateStaticIp - status code:"+xhr.status+"" ); 
             console.log("updateStaticIp - reponse:"+xhr.responseText+"" ); 
            }
        }
        xhr.send(params);
    }
    
    window.addEventListener("DOMContentLoaded", (event) => {

        // Get the modal
        modal = document.getElementById("myModal" );
        modalPwd = document.getElementById("myModalpwd" );
      // Get the <span> element that closes the modal
        span = document.getElementsByClassName("close" )[0];
        // When the user clicks on <span> (x), close the modal
      span.onclick = function() {
        modal.style.display = "none";
      }
      // When the user clicks anywhere outside of the modal, close it
      window.onclick = function(event) {
        if (event.target == modal) {
          modal.style.display = "none";
        }
      }



      let toApDelay = 50;
      try{
        if(!!myVars){
          document.getElementById("ssid" ).value = myVars["ssid"];
          if(myVars["wifipass"] == "kept for me")
          {
            document.getElementById("wifipass").setAttribute("placeholder", myVars["wifipass"]) 
            document.getElementById("wifipass" ).value = "";
          }
          else
            document.getElementById("wifipass" ).value = myVars["wifipass"];
            
          document.getElementById("hostname" ).value = myVars["hostname"];
          document.getElementById("login" ).value = myVars["login"];
          document.getElementById("pwd" ).value = myVars["pwd"];
          document.getElementById("version" ).innerHTML = myVars["version"];
          document.getElementById("macAddress" ).innerHTML = myVars["macAddress"];

          //document.getElementById("useStaticIp").checked = (myVars["useStaticIp"] === "true" || myVars["useEth"] == 1) ;
          if (myVars["useStaticIp"] === "true" || myVars["useStaticIp"] == 1)
          {
            document.getElementById("StaticIpChoice1").checked = true;
            //allowStaticIp(true);
          }
          else
            document.getElementById("StaticIpChoice2").checked = true
            
          document.getElementById("ip").value = myVars["ip"];
          document.getElementById("mask").value = myVars["mask"];
          document.getElementById("gateway").value = myVars["gateway"];;
          document.getElementById("dns").value = myVars["dns"];

          if (myVars["useStaticIp_eth"] === "true" || myVars["useStaticIp_eth"] == 1)
          {
            document.getElementById("StaticIpChoice1_eth").checked = true;
            //allowStaticIp(true);
          }
          else
            document.getElementById("StaticIpChoice2_eth").checked = true;

          if (myVars["isWt32"] === "true" || myVars["isWt32"] == 1)
          {
            document.getElementById("isWt32").checked = true;
            //allowStaticIp(true);
          }
          else
            document.getElementById("isWt32").checked = false;
            
          document.getElementById("ip_eth").value = myVars["ip_eth"];
          document.getElementById("mask_eth").value = myVars["mask_eth"];
          document.getElementById("gateway_eth").value = myVars["gateway_eth"];;
          document.getElementById("dns_eth").value = myVars["dns_eth"];
          
          document.getElementById("myPhy_addr").value = myVars["myPhy_addr"];
          document.getElementById("phy_addr_default").value = myVars["phy_addr_default"];
          

          if(myVars["useEth"] === "true" || myVars["useEth"] == 1)
          {
            //document.getElementById("useEth").checked = (myVars["useEth"] === "true" || myVars["useEth"] == 1);
            document.getElementById("useEth_True" ).checked = true;
          }
          else
          {
            document.getElementById("useEth_False" ).checked = true;
          }
         

          
          toApDelay = myVars["delayBeforeApDown"]/1000;
          document.getElementById("timer").value = toApDelay; 

          //if(myVars["hostname"]=="")
          if( editingPwd == "" || ( editingPwd != "" && document.getElementById("tokenField" ).value == editingPwd )) 
          {
            if( myVars["editingPwdSet"] != "1")
            {
              document.getElementById("resultCheckPwd").innerHTML="Define this doorkeeper edition password <br> <small>or if already done, close and click the edit button to define the hostname</small>"
              showModal("myModalpwd");
            }
            else
            {
              console.log("mot de passe deja defini.");
            }
          }
          }else
          console.log("myVars not loaded... grrrr" );
        }
        catch (err)
        {
          console.log("Error: got for myVars: "+JSON.stringify(err));
        }
       
        let goingOff = document.getElementById("timer");
        let ApCounterId = setInterval(function() {
          //if(document.getElementById("timer").innerHTML > 0)
          if(toApDelay > 0)
          {
            //console.log("interval on action:"+document.getElementById("timer" ).innerHTML);
            toApDelay = toApDelay-1;
            var minToApDelay = Math.floor(toApDelay / 60);
            var secToApDelay = toApDelay - minToApDelay * 60;
            document.getElementById("timer").innerHTML = minToApDelay+":"+secToApDelay;
          }
          else{
            //console.log("interval on action: should stop " );
            document.getElementById("timer").innerHTML="Access point has been turned off<br>Check your wifi and reload page when you hoocked back on this Access Point.";
            clearInterval(ApCounterId);
          }
        }, 1000);
     


        try{
          //console.log("myWifis :"+myWifis);
          let wifiList="";
          if(!!myWifis){
           
            for (i in myWifis)
            {
              wifiList += "<tr><td>"+myWifis[i].id+"</td><td onclick='setSsdiText(this.innerHTML)'>"+myWifis[i].ssid+"</td><td>"+myWifis[i].rssi+"</td><td>"+myWifis[i].encryption+"</td></tr>"
            }
            if(myWifis["ssid"] != "" )
            {
              document.getElementById("restartAfterWifi" ).checked = true;
            }
          }
          document.getElementById("listWifiBody").innerHTML=wifiList;
        }catch(errwifi)
        {
          console.log("Error: got for myWifis: "+JSON.stringify(errwifi)+", error:"+errwifi);

        }


        document.getElementById("ssid").setAttribute("disabled", true);
          document.getElementById("wifipass").setAttribute("disabled", true);
          document.getElementById("hostname").setAttribute("disabled", true);
          document.getElementById("login").setAttribute("disabled", true);
          document.getElementById("pwd").setAttribute("disabled", true);
          document.getElementById("submitwifi").setAttribute("disabled", true);
          document.getElementById("submitCredentials").setAttribute("disabled", true);
          document.getElementById("submitUpdate").setAttribute("disabled", true);
          document.getElementById("submitStaticIp").setAttribute("disabled", true);
          document.getElementById("showUpdatePwdBtn").setAttribute("disabled", true);
          //document.getElementById("useStaticIp").setAttribute("disabled", true);
          document.getElementById("useEth").setAttribute("disabled", true);
          document.getElementById("isWt32").setAttribute("disabled", true);
          
          document.getElementById("ip").setAttribute("disabled", true);
          document.getElementById("mask").setAttribute("disabled", true);
          document.getElementById("gateway").setAttribute("disabled", true);
          document.getElementById("dns").setAttribute("disabled", true);
          document.getElementById("wt32Conf").setAttribute("disabled", true);
          document.getElementById("staticIpWifiConf").setAttribute("disabled", true);
          document.getElementById("staticIpEthConf").setAttribute("disabled", true);

          
          //document.getElementById("useStaticIp_eth").setAttribute("disabled", true);
          document.getElementById("ip_eth").setAttribute("disabled", true);
          document.getElementById("mask_eth").setAttribute("disabled", true);
          document.getElementById("gateway_eth").setAttribute("disabled", true);
          document.getElementById("dns_eth").setAttribute("disabled", true);
          
          document.getElementById("myPhy_addr").setAttribute("disabled", true); 
          
          

          
       
          
       

    });  
  </script>
</head>
<body>
  <div class="topnav">
    <h1>
    <svg style="background-color:white;" xmlns:dc="http://purl.org/dc/elements/1.1/"
   xmlns:cc="http://creativecommons.org/ns#"
   xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
   xmlns:svg="http://www.w3.org/2000/svg"
   xmlns="http://www.w3.org/2000/svg"
   xmlns:xlink="http://www.w3.org/1999/xlink"
   xmlns:sodipodi="http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd"
   xmlns:inkscape="http://www.inkscape.org/namespaces/inkscape"
   version="1.1"
   id="svg2"
   width="40.959999"
   height="40.959999"
   viewBox="0 0 40.959999 40.959999"
   sodipodi:docname="logoIanitorV2.svg"
   inkscape:version="0.92.4 (5da689c313, 2019-01-14) ">
  <metadata
     id="metadata8">
    <rdf:RDF>
      <cc:Work
         rdf:about="">
        <dc:format>image/svg+xml</dc:format>
        <dc:type
           rdf:resource="http://purl.org/dc/dcmitype/StillImage" />
        <dc:title></dc:title>
      </cc:Work>
    </rdf:RDF>
  </metadata>
  <defs
     id="defs6" />
  <sodipodi:namedview
     pagecolor="#ffffff"
     bordercolor="#666666"
     borderopacity="1"
     objecttolerance="10"
     gridtolerance="10"
     guidetolerance="10"
     inkscape:pageopacity="0"
     inkscape:pageshadow="2"
     inkscape:window-width="933"
     inkscape:window-height="480"
     id="namedview4"
     showgrid="false"
     inkscape:zoom="5.7617189"
     inkscape:cx="2.2562711"
     inkscape:cy="20.48"
     inkscape:window-x="183"
     inkscape:window-y="52"
     inkscape:window-maximized="0"
     inkscape:current-layer="svg2" />
  <image
     width="40.959999"
     height="40.959999"
     preserveAspectRatio="none"
     style="image-rendering:optimizeQuality"
     xlink:href="data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAIAAAACACAYAAADDPmHLAAAABmJLR0QA/wD/AP+gvaeTAAAACXBI
WXMAAC4jAAAuIwF4pT92AAAAB3RJTUUH5goWFi4xcXmUCQAAABl0RVh0Q29tbWVudABDcmVhdGVk
IHdpdGggR0lNUFeBDhcAAAvsSURBVHja7Z15jCRVHcc/v5nZOdh1uYZdFhaUKHKIMKggoETBqERU
IETBCIIKRvlDVIy3gKICHqCJiagsxBA5BA3xYlUUWI4oQoIGFuTY5TBLgF3Ynb1mZmf65x/16/Cs
reqq6q7u6p75fZOXqe55Va/qve/7Xe/3qkVVccxdDPTCTYpIx1mqqjIXCNDnc2BuwwlQnVQbE5EV
IjIlIneJyJ6V3Ecv2ABxFaCqIiJDwKuARcAEsB4YBwTYBuwADAEjdlo/sBOwGzADPA0osNCucX2n
VICIHA/8DNgj+Ppe4FRVXd1pXdf1xQYqLJfZAEu8xM6ThNJnpX58CHBb7PqfAQ5o07OcDmxIeCYF
VgL7drJv+7p85veJyHHAtbF/7Q1MaACb4YiIhH8TCF+zoiYhknA5sFJE/iUinxWRxSU8y6iIXAlc
aVInCQcAvxWRg+e0BDAx/UXgiZSZ8iAwFJvpC4GRnNcXYJ6J/pOA1Snt1MsUcCPwhiaf5612z7WM
duplNXACMND2vu6ygd8VuBTYnKOT+tukXs4GbmpwD38Ejsp57YXA180+0YJlEvgr8Dagb1YTwAy1
C1J041rge8DB8f+1gwDB968APgbckTJzfwksaXDdY4D7zeDUFsoW4Apg0awkAHAs8FjCg99vBtNw
1mCVRYAGdQ4yO2Q6dg/rgU+HM9Q8jctzSrG8pQY8Ahw3awhgM2xZwux6EDg5btG3gwRFrwXsa0QI
z1sD7Gx2xV7AigK6vmjZBJzT8wQwa/fh2MO9CHw8S99VSYCYeF9pg7+HfbfEfHltc5kAPtmzBLDZ
PR57qOuBxU0OXF/dWgaGzZDcIR4bCCWKrYGMtUIk8yJGrY0FwN86MPih6jmm5whg+rIWE2mntjhz
b7NgzkCB8weA62LXORmY3+RzXd5GsZ9WHi0yaSonAPDlhAc4qMD5ArzL/HFNIMHSlAhgPfIXGmo7
p/j+42ZxH1rgvt5plrpWUJb1BAHMxQtvfAWwU4GBf595BY064x3mdw+aKhi2SN8QML8eKDKx/aEc
nbscODzj3gaBuysa/LoEPayrCQCcmXDj83OeeyBwZwc68pvAQyn/+xWwe4PZP1khARS4qmsJAByd
1kEZ5/Wb1JhMYPxl9YWaMjow1u5bgGtsNTHuoZyZcJ/XVjz4CjzXKCBVGQHMJ34hb+cH540Cf04I
i/4gHg0rmwDBdV9rkb64YXd64HGMmRtYNQFmgFO6igCmt/8UC5YszRoEC7TEjbO7gAPbEeHLcY23
m7Fav9aHTe+PAEfYIpF2QflJtxHgE8HNTQNH5pi1rwOejTH7gmYXQkoMFo0AV5unsSSIK3yqAtcv
rdzaNQSwtfow0HNJjhW4JOv2+HbH+Ateb7RORpNwF3XJ4CvwQLNLx+3ICr7Q4vyYZX1BWsZtSrbv
BuA9qnpPNyWnqOraINFkZ+CwLrq9HS36OV5pQogZT6EFfXTBqN4W4IhOrfI1ed1Bi/xpF5UXgP26
ISXswmCvwXJVvTMjTer0kIvA2ar6925OU1PVKWBjl93WPLNJqksJA/YJkh9qwJsy6i8G1gUs/lG7
Eknb5OU820USYLzZ5yxTAnw02Gdwi6rel1H/q8AudrzaPvcELKH0n7Nhf0cpRqCI9AFnBF/9PKP+
Usu9q+McVd3UY3s7Nsc+Pwk8A7zaPIbBsnln9tVGWw7e3dY46gQ4oDICWEx8bzt+HvhDRv3P22IN
wN2qupzew3Ts86WqeoWIjNoaxnstcWQ/G6i+JgZ83KTjauB2Szh5xlTn720BjGDVszICnBAcX6Oq
2xrM/iHgtOCrb9CbmIwN1qrAXVwBrBCRQeCVwFHAkcCbLXchbdfRVvPpVwL/tiSTR1R1OqEfnypD
DZRFgGOD499l1H2/Ze3Udf+tPUoAjR3/N8VjeMzKL0RkwPrnuBSJcpaqXpuz/ZXWbktb2Fo2AkVk
DxNzmB+f5cZ9MDi+Wnv3BQWhKzhl0css43EauLmBSnmgQPsPmbdFEHLvPAECPQRwj6pOZhiLxwRf
3UTvYm0gBV4qYPT9pYE0Hi3Q/qpADdXMBa+EAIcGx3dk1D0kEP/PEuW69yrWBccbCriQq0zXx9Fv
kdS8eI2dUydArSobYN/g+OGMukcGx7f3sPiHl3MdxAy9L4jIjUTLxy8Bm1S1liAFB1MGS4j2EF4Z
qz9AtNlkqU2gMeBw4PW8vLm1aRVQNgEey6gbMvwBehvrbCD7iJaMzyYKhm0yP32NiDxhovoOc3en
gP0bqIvDRGRYVSdEZC/gPPMg9iQKmg2mSO2BZgnQaki0n5fTtmpk5PpZfKAevjypU+8UaNP1DzJR
nidUuwEYs/O+T+N9gG+0eqex/Va0wtlN7Q4FLwrY/Jyqbs6ov09w/HiPS4BazBVshEFgWkQWEO0/
SMOwuckQ5fq1XUW2SoAdguM8odwdY1Z0L2MoMMLykGUrcCJRrmSqowS8O3Az8xJgoxmFHSfASCyK
lYX5wfGWwNDRpNLAnSxUv9XzgzrhGsd4zmeuq8oRovh9FmbMXR4oEOS5jyhEXCkBJgpKjC09OOuH
66+eUdUnzGLPQ7wBorWSnwJfi7mQYTRxBVH6eY1oI0seAqwFPmcGZseNwDHKyccvZV08rxHYQhtr
CN5MQv5NoTPAacF5+xNtcZsMjL9LgAVBnVPI93KJiyt7SZSqNu3KicjCHpQASwhe8GRL2DfnkAIC
jIpIv4j0qeojNsAfMc/oA6r6pdiS+HBOCdDfygNV9qpYVR2nN7GXBXqSDNuGBFDVmeD5a8ANIvKb
lNXT+QVI2R0EyHq5YtF3/qZdr13vDs7Z3olES7V17JJzpu6S0mba0vmCnLe9eyvP7K+KLY6TYp/z
qrKdCraTV7TvZuHi3lIBRSRHq/Xb3N6CNk22vC7mSD3Q5BKgGuSdRLWYWskiYd4cycFWJrIToHXU
Wjm5ARE254wxzKOFBFQnQMUEKEEFaCv34ARoHePNECXIhUiTABtyDuxW8kVhnQBtwtM5RfVME8TK
Q4BNToBq8WROAhQdpI05LfsNSZlHToDOYXXO2T3TQIcnYVtOAqxq5eadAK3jUZJX9+JIy5aWBhIj
a4VvG9n7MJwA7YSqPg/ckEMNFA3UbGtAmjruI/r9AidAxbiY7ISMyYJjMJFhN0wB3246D8AJUKoU
eA74bgOrvb6zN+nc6QYEaBQLWAHc0uq9OwHKwzIaJ7puLUiqabbfgh5e6zutWP9OgPKlwIRJgeky
CBDEApKwXFVvK+O+nQDlkmAZ6Xv//k+f51zCXZ/w3QtEeYU4AboMItIPnAX8J8EGiEuAPOv9LyV4
Bl9R1ZWzigA9lBaeVX+Jqq4hel3O2kYSIKdb+GKMRNcBV5XZ9y4BysVRlux6L3BubNZvTZAKWQi3
oP8DOLcMw69nCNDqTK+gvR8SpYwN2Zs+zreZXmP7dwvuWs8SbnC9p+zcZ4AzVHV92c/sEqBcLCH6
gYwx+3wZUZTwKbbfOX0s0WbaRjP6bqK3rp+nqo+2hfStbtEPZ0iRrOCwblmzOu2a8ftqR3ux616k
qudn9MVCog0htbLFehGUmhRatnju0rTwPDhLRGaAH6vqupS2umJfhKuA9qmCC4FLRGRxhp6v1s4q
UwWUMdvaIY3a3VaOvvg18C2ipeOJKkV+W1VA2TuDehk5+uIMokjf46r60KyQAM0agbNRAnRTX7gN
4HACOJwADieAwwngcAI4nAAOJ4DDCeBwAjgBHE4Ax9xFV7wlLG2VsGiCRt5FlU635xLA4RKgTMkw
W9pzCeBwAjhcBaSih7OCXQI4XAK0PPPKql91ey4BHE4AxxxQAXPRj+71vnAJ4CrA4SqgQ9a8bw3r
vr5wCeAqwOEEcDgBHE4AhxPA4QRwOAEcTgCHE8DhBHA4ARxOAIcTAERkvnfXHCaAqm727nIV4HAC
OJwAjlmDzJdFe6Zv5XgQOFxVC//wpIiIqqqISMrkHx7w/u16TDUz+Ga4a/g3hhlgcx4V4NZ/N4vw
6JfHRpJmuYjMsx+zRCIsEpHB8BdMWv69AIcbgQ4ngMMJ4HACOJwADieAwwng6Bn8D5NebmolOOhJ
AAAAAElFTkSuQmCC
"
     id="image10"
     x="0"
     y="0" /></svg>
    Ianitor DoorKeeper Wi-Fi Manager</h1>
    <button  onclick="edit();" style="color:black">edit</button>
  </div>
  <div class="content">
    <div class="card-grid">
      <div class="card" style="background-color: lightgray;">
        <div style="background-color: whitesmoke;">
          <label for="version">firmware version: </label><span id="version"></span></div>
        <hr style="background-color: whitesmoke;">
        <label for="macAddress">macAddress: </label><span id="macAddress"> +ETH.macAddress()+ </span></div>
        
        <hr style="background-color: whitesmoke;">
        <span>Click on desired SSID</span>

        <table>
          <thead><tr>
              <th scope="col">id</th>
              <th scope="col">SSID</th>
              <th scope="col">RSSI</th>
              <th scope="col">Encryption</th>
            </tr>
          </thead>
          <tbody id="listWifiBody"></tbody>
        </table>
      </div>
      <div class="card">
        <form action="/" method="GET">
          <p>
            <label for="timer">Access point going off in approximatively:</label>
            <div id="timer" >50</div><br>
            <input type="submit"id="onForXmin" name="onForXmin" value="maintain access point On"/>
          </p>
        </form>
        <hr>
        <form action="/setwifi" method="POST" >
          <h2>
          Wifi / ethernet settings 
          </h2>
          <p>
            <label for="ssid">SSID</label>
            <input type="text" id ="ssid" name="ssid"><br>
            <label for="wifipass">Password</label>
            <input type="text" id ="wifipass" name="wifipass"><br>
            <label>leave blank to keep the active one</label><br>
            <br>
            <input type="checkbox" id ="noWifiPassBox" name="noWifiPassBox" value="noPwd"><label for="noWifiPassBox">no wifi password</label><br>

            <br>
            <p>
              <fieldset id="wt32Conf" disabled>
                  <legend><strong>board is wt32 :</strong></legend>
                  <div>
                  <input type="hidden" name="isWt32" value="0">
                      <input type="checkbox" id="isWt32" name="isWt32" value="1" onchange="allowWt32edition( document.getElementById('isWt32').checked );" /> <label for="isWt32">is WT32</label>

                  </div>
                  <div>
                    Prefer Ethernet (cable rj45) if is wt32
                  </div>
                  <div>
                    <input type="radio" id="useEth_True" name="useEth" value="1"/>
                    <label for="useEth_True">Yes</label>
              
                    <input type="radio" id="useEth_False" name="useEth" value="0"/>
                    <label for="useEth_False">No</label>
                  </div>
                  
                  <hr>
                
                  <div>phy addr:</div>
                    <label for="myPhy_addr">phy_addr: </label><input type="number" id="myPhy_addr" name="myPhy_addr" value="-1">
                    <label for="phy_addr_default">phy_addr_default: </label><input type="number" id="phy_addr_default" name="phy_addr_default" value="-1" disabled>
                </fieldset>
                
                <!--input type="checkbox" id="useEth" onChange="" onchange="document.getElementById('useEth').value = document.getElementById('useEth').checked;" name="useEth" value="1"> Prefer Ethernet (cable rj45), if wt32 </input-->
              </p>
            <hr>
            <input type="checkbox" id="restartAfterWifi" name="restartAfterWifi">restart after wifi setting</input>
            <input type ="submit" id="submitwifi" value ="Submit">
          </p>
        </form>
        <hr>
        <form action="/" method="POST">
          <p>
            <h2>Ianitor server credentials</h2>
            <label for="hostname">Hostname</label><input type="text" id ="hostname" name="hostname" value="" placeholder="Server hostname"><br>
            
            <label for="login">login</label><input type="text" id ="login" name="login" value="" placeholder="login - doorkeeper"><br>
            <label for="pwd">pwd</label><input type="text" id ="pwd" name="pwd" value="" placeholder="Pwd - leave Blank to keep the active one"><br>
            
          </p>
          
          <p>
            <input type ="submit" id="submitCredentials" value ="Submit">
          </p>
        </form>
        <hr>
        <form action="/updateStaticIp" method="POST">
         <p>
         <h2>Static ip</h2>
          <fieldset id="staticIpWifiConf" disabled>
              <legend>Use Static Ip for WIFI:</legend>
              <div>
                <input type="radio" id="StaticIpChoice1" name="useStaticIp" value="true" onChange="allowStaticIp(document.getElementById('StaticIpChoice1').checked);" />
                <label for="StaticIpChoice1">Yes</label>
          
                <input type="radio" id="StaticIpChoice2" name="useStaticIp" value="false" onChange="allowStaticIp(document.getElementById('StaticIpChoice1').checked);"/>
                <label for="StaticIpChoice2">No</label>
              </div>
              
            
               <!--input type="checkbox" id ="useStaticIp" name="useStaticIp" value="" onChange="document.getElementById('useStaticIp').value = document.getElementById('useStaticIp').checked; allowStaticIp(document.getElementById('useStaticIp').checked);" > use Static IP </input><br-->
             <hr>
              <p>
                <label for="ip">IP</label>
                <input type="text" id ="ip" name="ip" value="" placeholder="192.168.x.x"/><br>
                <label for="mask">Subnet Mask</label>
                <input type="text" id ="mask" name="mask" value="" placeholder="255.255.y.y"/><br>
                <label for="gateway">Gateway</label>
                <input type="text" id ="gateway" name="gateway" value="" placeholder="192.168.x.1"/><br>
                <label for="dns">DNS</label>
                <input type="text" id ="dns" name="dns" value="" placeholder="8.8.8.8"/><br>
              </p>
          </fieldset>
          <fieldset id="staticIpEthConf" disabled>
              <legend>Use Static Ip for ETH:</legend>
              <div>
                <input type="radio" id="StaticIpChoice1_eth" name="useStaticIp_eth" value="true" onChange="allowStaticIp_eth(document.getElementById('StaticIpChoice1_eth').checked);" />
                <label for="StaticIpChoice1_eth">Yes</label>
          
                <input type="radio" id="StaticIpChoice2_eth" name="useStaticIp_eth" value="false" onChange="allowStaticIp_eth(document.getElementById('StaticIpChoice1_eth').checked);"/>
                <label for="StaticIpChoice2_eth">No</label>
              </div>
            <hr>
              <p>
                <label for="ip_eth">IP eth</label>
                <input type="text" id ="ip_eth" name="ip_eth" value="" placeholder="192.168.x.x"/><br>
                <label for="mask_eth">Subnet Mask eth</label>
                <input type="text" id ="mask_eth" name="mask_eth" value="" placeholder="255.255.y.y"/><br>
                <label for="gateway_eth">Gateway eth</label>
                <input type="text" id ="gateway_eth" name="gateway_eth" value="" placeholder="192.168.x.1"/><br>
                <label for="dns_eth">DNS eth</label>
                <input type="text" id ="dns_eth" name="dns_eth" value="" placeholder="8.8.8.8"/><br>
              </p>
          </fieldset>
          <p>
             <input type ="submit" id="submitStaticIp" value ="Submit">
          </p>
        </form>
        <hr>
        <form action="/restart" method="" style="">
          <h2>
          restart now
          </h2>
          <p>
            <input type ="submit" id="submitRestart" value ="restart now">
          </p>
        </form>
        <hr>
        <div style="">
          <h2>
          Change edition password
          </h2>
          <p>
            <input type ="button" id="showUpdatePwdBtn" onclick="showModal('myModalpwd');" value ="update password"/>
          </p>
        </div>
        <hr>
        <form action="/update" method="" style="">
          <h2>
          update this groom
          </h2>
          <p>
            <input type ="submit" id="submitUpdate" value ="to update page">
          </p>
        </form>        
      </div>
    </div>
  </div>


<!-- The Modal -->
<div id="myModal" class="modal">

  <!-- Modal content -->
  <div class="modal-content">
    <span class="close">&times;</span>
    <p id="resultCheck"></p>
    <p>enter password for edition</p>
    <input type="password" id="editingPwd" placeholder="editing password"/>
    <button onclick="checkPassword();" style="color:black">check password</button>
  </div>

</div>
<div id="myModalpwd" class="modal" width="100%" height="100%">

  <!-- Modal content -->
  <div class="modal-content">
    <span id="closeModalPwd" class="close" onClick="closeModal('myModalpwd');">&times;</span>
    <p id="resultCheckPwd"></p>
    <p>Enter new password for edition</p>
    <input type="password" id="updatePwd" placeholder="new password"/>
    <button onclick="updatePassword();" style="color:black">update password</button>
  </div>

</div>
<input id="tokenField" type="hidden" />
</body>
</html>
)";



/******* ----- pages web  -------*/
/************************************************************************************/


const char* wl_status_to_string(uint8_t status) {
  switch (status) {
    case WL_NO_SHIELD: return "WL_NO_SHIELD";
    case WL_IDLE_STATUS: return "WL_IDLE_STATUS";
    case WL_NO_SSID_AVAIL: return "WL_NO_SSID_AVAIL";
    case WL_SCAN_COMPLETED: return "WL_SCAN_COMPLETED";
    case WL_CONNECTED: return "WL_CONNECTED";
    case WL_CONNECT_FAILED: return "WL_CONNECT_FAILED";
    case WL_CONNECTION_LOST: return "WL_CONNECTION_LOST";
    case WL_DISCONNECTED: return "WL_DISCONNECTED";
    default: return "WL_Unknown";
  }
}

String get_wifi_status(int status){
    switch(status){
        case WL_IDLE_STATUS:
        return "WL_IDLE_STATUS";
        case WL_SCAN_COMPLETED:
        return "WL_SCAN_COMPLETED";
        case WL_NO_SSID_AVAIL:
        return "WL_NO_SSID_AVAIL";
        case WL_CONNECT_FAILED:
        return "WL_CONNECT_FAILED";
        case WL_CONNECTION_LOST:
        return "WL_CONNECTION_LOST";
        case WL_CONNECTED:
        return "WL_CONNECTED";
        case WL_DISCONNECTED:
        return "WL_DISCONNECTED";
    }
}
//Serial.println(wl_status_to_string(WiFi.status()));









// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
//IPAddress local_IP(10,10,4,2);
//IPAddress gateway(10,10,4,1);
//IPAddress subnet(255,255,255,0);
bool makeAPOn = false;
bool loadPortier = false;
bool withSocketFallToWifi = true;
bool maintenanceOn = false;


// Search for parameter in HTTP POST request
const char* PARAM_INPUT_1 = "ssid";
const char* PARAM_INPUT_2 = "wifipass";
const char* PARAM_INPUT_11 = "wifimulti";
const char* PARAM_INPUT_3 = "ip";
const char* PARAM_INPUT_4 = "gateway";
const char* PARAM_INPUT_12 = "dns";
const char* PARAM_INPUT_13 = "mask";
const char* PARAM_INPUT_14 = "useStaticIp";
const char* PARAM_INPUT_15 = "useEth";
const char* PARAM_INPUT_16 = "myPhy_addr";
const char* PARAM_INPUT_17 = "noWifiPassBox";
const char* PARAM_INPUT_18 = "useStaticIp_eth";
const char* PARAM_INPUT_19 = "ip_eth";
const char* PARAM_INPUT_20 = "gateway_eth";
const char* PARAM_INPUT_21 = "dns_eth";
const char* PARAM_INPUT_22 = "mask_eth";
const char* PARAM_INPUT_23 = "isWt32";
const char* PARAM_INPUT_5 = "hostname";
const char* PARAM_INPUT_6 = "login";
const char* PARAM_INPUT_7 = "pwd"; 
const char* PARAM_INPUT_8 = "withSocketFallToWifi";
const char* PARAM_INPUT_9 = "myId";
const char* PARAM_INPUT_10 = "gpioLed";

// Search for parameter in HTTP GET request
const char* PARAM_GET_1 = "onForXmin";
bool onForXminBool = false;
const long XminDelay = 10*60*1000;

////Variables to save values from HTML form
String APname = "Ianitor Door Keeper";
String ssid = "wifiSSID";
String wifipass = "thiswifipass";
String ip = "";
String gateway = "";
String dns = "8.8.4.4";
String mask = "";
String ip_eth = "";
String gateway_eth = "";
String dns_eth = "8.8.4.4";
String mask_eth = "";

IPAddress myIp;
IPAddress myGW;
IPAddress mySN;
IPAddress myDNS;
bool useStaticIp = false;
bool useStaticIp_eth = false;
String hostname = "192.168.43.187.9";
String socketHostname = "192.168.43.187.10";
String updateHostname = "https://doc.ianitordoorkeeper.com/firmwares/esp32/";
bool firmwareUpdated = false;
int hostnamePort = 80;
String login = "portier2-AAAA-9";
String pwd ="J35u1sUnP0rT1e4-AAAA-9";
String myId = "fakeId";
String amIRegistered = "0";
String ipExt = "1";
String ipInt = "1";
String sendHttpCommandResp = "notSet";
String editingPassword = "DevraitEtr3Aff3ct3AChaineVideParLesPreferencesS1NonInit1alise!";

//// File paths to save input values permanentlys
const char* ssidPath = "/ssid.txt";
const char* wifipassPath = "/pass.txt";
const char* ipPath = "/ip.txt";
const char* gatewayPath = "/gateway.txt";

String wifiToJSon = "";

const int doors_to_relay_size = 19;
//int doors_to_relay[11] = {17, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33};
int gpioTrad[doors_to_relay_size] =       {2, 4 , 5, 12, 13, 14, 15, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33};
int doors_to_relay[doors_to_relay_size] = {2, 4 , 5, 12, 13, 14, 15, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33};
//id portes                                0  1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18
int gpioLed = 0; // doit etre appele apr doors_to_relay -> 2 dans doors_to_relay[]
String gpioLedStr = "notDefined";
int blinkDELAY = 200;
// setting PWM properties
const int freq = 1;
const int freq_connected = 255;
const int freq_disconnected = 0;
const int freq_ApOn = 51;
const int freq_tryingWifi = 128;
const int ledChannel = 0;
const int ledDuty = 1;
// use 12 bit precision for LEDC timer
#define LEDC_TIMER_12_BIT  12
const int resolution = LEDC_TIMER_12_BIT; //aucune idee de ce que c est mais il faut qu il soit bas: 8 ne va pas.


const int SOCKET_CONNECTED = 1;
const int SOCKET_DISCONNECTED =0;
const int ACCESS_POINT_ON = 2;
const int TRYING_WIFI = 3;

int delayOn = 700; 

bool isRegistred = false;


const char echo_org_ssl_ca_cert[] PROGMEM = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIEZTCCA02gAwIBAgIQQAF1BIMUpMghjISpDBbN3zANBgkqhkiG9w0BAQsFADA/\n" \
"MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\n" \
"DkRTVCBSb290IENBIFgzMB4XDTIwMTAwNzE5MjE0MFoXDTIxMDkyOTE5MjE0MFow\n" \
"MjELMAkGA1UEBhMCVVMxFjAUBgNVBAoTDUxldCdzIEVuY3J5cHQxCzAJBgNVBAMT\n" \
"AlIzMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuwIVKMz2oJTTDxLs\n" \
"jVWSw/iC8ZmmekKIp10mqrUrucVMsa+Oa/l1yKPXD0eUFFU1V4yeqKI5GfWCPEKp\n" \
"Tm71O8Mu243AsFzzWTjn7c9p8FoLG77AlCQlh/o3cbMT5xys4Zvv2+Q7RVJFlqnB\n" \
"U840yFLuta7tj95gcOKlVKu2bQ6XpUA0ayvTvGbrZjR8+muLj1cpmfgwF126cm/7\n" \
"gcWt0oZYPRfH5wm78Sv3htzB2nFd1EbjzK0lwYi8YGd1ZrPxGPeiXOZT/zqItkel\n" \
"/xMY6pgJdz+dU/nPAeX1pnAXFK9jpP+Zs5Od3FOnBv5IhR2haa4ldbsTzFID9e1R\n" \
"oYvbFQIDAQABo4IBaDCCAWQwEgYDVR0TAQH/BAgwBgEB/wIBADAOBgNVHQ8BAf8E\n" \
"BAMCAYYwSwYIKwYBBQUHAQEEPzA9MDsGCCsGAQUFBzAChi9odHRwOi8vYXBwcy5p\n" \
"ZGVudHJ1c3QuY29tL3Jvb3RzL2RzdHJvb3RjYXgzLnA3YzAfBgNVHSMEGDAWgBTE\n" \
"p7Gkeyxx+tvhS5B1/8QVYIWJEDBUBgNVHSAETTBLMAgGBmeBDAECATA/BgsrBgEE\n" \
"AYLfEwEBATAwMC4GCCsGAQUFBwIBFiJodHRwOi8vY3BzLnJvb3QteDEubGV0c2Vu\n" \
"Y3J5cHQub3JnMDwGA1UdHwQ1MDMwMaAvoC2GK2h0dHA6Ly9jcmwuaWRlbnRydXN0\n" \
"LmNvbS9EU1RST09UQ0FYM0NSTC5jcmwwHQYDVR0OBBYEFBQusxe3WFbLrlAJQOYf\n" \
"r52LFMLGMB0GA1UdJQQWMBQGCCsGAQUFBwMBBggrBgEFBQcDAjANBgkqhkiG9w0B\n" \
"AQsFAAOCAQEA2UzgyfWEiDcx27sT4rP8i2tiEmxYt0l+PAK3qB8oYevO4C5z70kH\n" \
"ejWEHx2taPDY/laBL21/WKZuNTYQHHPD5b1tXgHXbnL7KqC401dk5VvCadTQsvd8\n" \
"S8MXjohyc9z9/G2948kLjmE6Flh9dDYrVYA9x2O+hEPGOaEOa1eePynBgPayvUfL\n" \
"qjBstzLhWVQLGAkXXmNs+5ZnPBxzDJOLxhF2JIbeQAcH5H0tZrUlo5ZYyOqA7s9p\n" \
"O5b85o3AM/OJ+CktFBQtfvBhcJVd9wvlwPsk+uyOy2HI7mNxKKgsBTt375teA2Tw\n" \
"UdHkhVNcsAKX1H7GNNLOEADksd86wuoXvg==\n" \
"-----END CERTIFICATE-----\n";


// free RAM check for debugging. SRAM for ATmega328p = 2048Kb.
int availableMemory() {
    // Use 1024 with ATmega168
    int size = 2048;
    byte *buf;
    while ((buf = (byte *) malloc(--size)) == NULL);
        free(buf);
    return size;
}
String encripte(String toEncripte)
{
  return toEncripte;
}

void endTask(TaskHandle_t _task)
{
  vTaskDelete(_task);
}

bool ledIsOn = false;
int counterLed = 0;
int maxBlink = 3;
void ledState(int _ledState)
{
  //_ledState: SOCKET_CONNECTED, SOCKET_DISCONNECTED ,ACCESS_POINT_ON, TRYING_WIFI

// use 12 bit precission for LEDC timer
//#define LEDC_TIMER_12_BIT  12

    // calculate duty, 4095 from 2 ^ 12 - 1
  uint32_t valueMax = 255;
  //uint32_t thisDuty = (4095 / valueMax) * min(duty, valueMax);
  uint32_t thisDuty;
  
  switch(_ledState)
  {
    case SOCKET_DISCONNECTED:
    Serial.println("ledState(): setting to DISCONNECT");
      ledIsOn = true;
//        analogWrite(doors_to_relay[gpioLed],0);

        //ledcChangeFrequency(ledChannel, freq_disconnected, resolution);
         thisDuty = (4095 / valueMax) * freq_disconnected;

        ledcWrite(ledChannel, thisDuty);

    break;
    case SOCKET_CONNECTED:
        Serial.println("ledState(): setting to CONNECT");
              Serial.print("ledState(): running on core: ");
      Serial.println(xPortGetCoreID());
      
      Serial.print("/*/*/*/*/*/* POUR INFO: binary event = ");
      Serial.print(sIOtype_BINARY_EVENT);
      Serial.println("/*/*/*/*/*/*");
      
      //analogWrite(doors_to_relay[gpioLed],255);
      ledIsOn = true;
              Serial.println("ledState(): var 'ledIsOn' = true");

      //ledcChangeFrequency(ledChannel, freq_connected, resolution);
          //thisDuty = 1;
          //thisDuty = (4095 / valueMax) * freq_connected;
          thisDuty = 4095;

        Serial.print("ledState(): thisDuty = ");
        Serial.println(thisDuty);

      ledcWrite(ledChannel, thisDuty);
              Serial.println("ledState(): ledcWrite() passé");

//      ledc_set_freq(freq_connected);

//      if(inverted)
//      {
//        digitalWrite(doors_to_relay[gpioLed], HIGH);
//      }
//      else
//      {
//        digitalWrite(doors_to_relay[gpioLed], LOW);
//      }
    break;
    case ACCESS_POINT_ON:
        Serial.println("ledState(): setting to ACCESS_POINT");
          thisDuty = (4095 / valueMax) * freq_ApOn;

        //ledcChangeFrequency(ledChannel, freq_ApOn, resolution);
        ledcWrite(ledChannel, thisDuty);
        
    break;
    case TRYING_WIFI:
        Serial.println("ledState(): setting to TRYING_WIFI");
          thisDuty = (4095 / valueMax) * freq_tryingWifi;

      //ledcChangeFrequency(ledChannel, freq_tryingWifi, resolution);
      ledcWrite(ledChannel, thisDuty);

    break;
  }
  
}
void ledInit()
{
  // configure LED PWM functionalitites
  ledcSetup(ledChannel, freq, resolution);
  
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(doors_to_relay[gpioLed], ledChannel);
  
  ledcWrite(ledChannel, ledDuty);

  //pinMode(doors_to_relay[gpioLed], OUTPUT);
  //ledState(SOCKET_DISCONNECTED);
}
void notifyConnected(void *_isRegistred)//void * pvParameters )
{
      USE_SERIAL.printf("notifyConnected - call\n");
      Serial.print("notifyConnected running on core: ");
      Serial.println(xPortGetCoreID());
      int nbIteration = 0;
      //while(!(bool)&_isRegistred)// && nbIteration < maxIteSockDownBeforeAP)//&& socketIsConnected(socketIO)) 
      bool keepon = true;
      while(keepon)
      {
        Serial.print("notifyConnected running on core: ");
        Serial.println(xPortGetCoreID());
        Serial.print("notifyConnected isRegistered: ");
        Serial.println(isRegistred);
        Serial.print("notifyConnected !isRegistered: ");
        Serial.println(!isRegistred);
//        try{
          //while(!isRegistred && (nbIteration < maxIteSockDownBeforeAP)) //&& socketIsConnected(socketIO)) 
          if(!isRegistred && (nbIteration < maxIteSockDownBeforeAP))
          {// creat JSON message for Socket.IO (event)

                    nbIteration++;
                    DynamicJsonDocument doc(1024);
                    JsonArray array = doc.to<JsonArray>();
            
                    // add evnet name
                    // Hint: socket.on('event_name', ....
                    array.add("portierLogin");
            
                    // add payload (parameters) for the event
                    JsonObject param1 = array.createNestedObject();
                    //param1["req"] = "login";
                    param1["id"] =  DateTime.now();
                    param1["login"] = login; //"portier2-AAAA";//(uint32_t) now;
                    param1["pwd"] = pwd; //"J35u1sUnP0rT1e4-AAAA";
                    param1["version"] = version;
                    param1["ethOn"] = String(useEth);
                    param1["macEth"] = ETH.macAddress();
                    param1["macWifi"] = ESP.getEfuseMac();
                    param1["ipExt"] = ipExt;

    //                param1["idSocketCaller"] = "idclaller1";
    //                param1["idPorte"] = "1";
    //                param1["idPortier"] = "7445";
    //                param1["state"] = "1";
                    //param1["date"] = (uint32_t) now;
                    //param1["date"] = (uint32_t) messageTimestamp;
                    if (!DateTime.isTimeValid()) {
                      Serial.println("Failed to get time from server, retry.");
                      DateTime.begin();
                      delay(250);
                      if(!DateTime.isTimeValid())
                      {
                        param1["date"] = DateTime.now();
                      }else{
                        param1["date"] = (uint32_t) messageTimestamp;
                      }
                    } else {
                      param1["date"] = DateTime.now();
                    }
            
                    // JSON to String (serializion)
                    String output;
                    serializeJson(doc, output);
    
                    // Send event
                    
                    USE_SERIAL.printf("notifyConnected - isRegistered vaut ");
                    USE_SERIAL.println(isRegistred);
                    USE_SERIAL.printf("notifyConnected - essai ");
                    USE_SERIAL.print(nbIteration);
                    USE_SERIAL.print("/");
                    USE_SERIAL.println(maxIteSockDownBeforeAP);
                    USE_SERIAL.printf("notifyConnected - sending Event to login\n");
                    USE_SERIAL.println(output);
                    socketIO.sendEVENT(output);
                    ledState(SOCKET_DISCONNECTED);  // pas certain que l'on puisse faire cela: https://docs.espressif.com/projects/esp-idf/en/v4.2/esp32/api-guides/fatal-errors.html#guru-meditation-errors
                    delay(1000);
          }
          else{
            keepon = false;
            USE_SERIAL.println("notifyConnected - FINISHED");
            endTask(TaskNotifyConnected);
          }
//        }catch(const std:Exception error1)
//        {
//          USE_SERIAL.print("error found: ");
//          USE_SERIAL.println(error1.toString());
//
//        }
      }
}
void runTaskNotifyConnect()
{
      xTaskCreatePinnedToCore(
                    notifyConnected,   /* Task function. */
                    "TaskNotifyConnected",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &TaskNotifyConnected,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */                  
    delay(500);
}
String getWifis()
{
   int n = WiFi.scanNetworks();
    Serial.println("getWifis(): scan done");
    wifiToJSon = "[";

    if (n == 0) {
        Serial.println("getWifis(): no networks found");
    } else {
      Serial.print(n);
      Serial.println("getWifis(): networks found");
      for (int i = 0; i < n; ++i) {
        // Print SSID and RSSI for each network found
        wifiToJSon += "{\"id\":"+String(i+1)+",\"ssid\":\""+WiFi.SSID(i)+"\",\"rssi\":\""+WiFi.RSSI(i)+"\",\"encryption\":\""+((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*")+"\"}";
        if(i < (n-1))
          wifiToJSon += ",";
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.print(WiFi.SSID(i));
        Serial.print(" (");
        Serial.print(WiFi.RSSI(i));
        Serial.print(")");
        Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
      }
    }
     wifiToJSon += "]";
    Serial.println("getWifis(): json: ");
     Serial.println(wifiToJSon);
     return wifiToJSon;
}
void openDoor(int _localIdDoor, int _delay=delayOn) {
  Serial.print("openDoor: ");Serial.print(_localIdDoor);Serial.print(" - port:");Serial.print(doors_to_relay[_localIdDoor]);Serial.print(", delay: ");Serial.println(_delay);
  if(inverted)
  {
    digitalWrite(doors_to_relay[_localIdDoor], LOW);
    delay(_delay);
    digitalWrite(doors_to_relay[_localIdDoor], HIGH);
  }
  else
  {
    digitalWrite(doors_to_relay[_localIdDoor], HIGH);
    delay(_delay);
    digitalWrite(doors_to_relay[_localIdDoor], LOW);
  }
}

void triggerGPIO (int _gpio, int _delay=delayOn){
    Serial.print("triggerGPIO: ");Serial.print(_gpio);Serial.print(" - port:");Serial.print(gpioTrad[_gpio]);Serial.print(", delay: ");Serial.println(_delay);
  if(inverted)
  {
    digitalWrite(gpioTrad[_gpio], LOW);
    delay(_delay);
    digitalWrite(gpioTrad[_gpio], HIGH);
  }
  else
  {
    digitalWrite(gpioTrad[_gpio], HIGH);
    delay(_delay);
    digitalWrite(gpioTrad[_gpio], LOW);
  }
}

void pinInit() {
   /*
 * initialisation des pins
 *
 **/
  //for (int iDoor=0; iDoor < sizeof(doors_to_relay); iDoor++ )
  //for (int iDoor=0; iDoor < 18; iDoor++ )  //ca merde car sizeof ne donne pas la taille du tableau mais... 48!
  for (int iDoor=0; iDoor < doors_to_relay_size; iDoor++ )  //ca merde car sizeof ne donne pas la taille du tableau mais... 48!
  {
    if(isWt32)
    {
      if(doors_to_relay[iDoor]==2 || doors_to_relay[iDoor]==4 ||doors_to_relay[iDoor]==12 || doors_to_relay[iDoor]==14 || doors_to_relay[iDoor]==15 )
      {
         if(inverted)
        {
          if( doors_to_relay[iDoor] != doors_to_relay[gpioLed] )
          //https://community.home-assistant.io/t/esp8266-relay-active-low-how-to-prevent-triggering-on-boot-or-reset-solved/88279
            digitalWrite( doors_to_relay[iDoor], HIGH); 
        }
         
        pinMode(doors_to_relay[iDoor], OUTPUT); // set ESP32 pin to output mode
      }
    }
    else{
      if(inverted)
      {
        if( doors_to_relay[iDoor] != doors_to_relay[gpioLed] )
        //https://community.home-assistant.io/t/esp8266-relay-active-low-how-to-prevent-triggering-on-boot-or-reset-solved/88279
          digitalWrite(doors_to_relay[iDoor], HIGH); 
      }
       
      pinMode(doors_to_relay[iDoor], OUTPUT); // set ESP32 pin to output mode
    }
  }
//  Serial.println("pinInit: setting led OFF for disconnected");
//   ledState(SOCKET_DISCONNECTED);

}

void setupDateTime() {
  // setup this after wifi connected
  // you can use custom timeZone,server and timeout
  // DateTime.setTimeZone("CST-8");
  // DateTime.setServer("asia.pool.ntp.org");
  // DateTime.begin(15 * 1000);
  // from
  /** changed from 0.2.x **/
  // DateTime.setTimeZone("CST-8");
  DateTime.setTimeZone("CET-1CEST,M3.5.0,M10.5.0/3");
  
  // this method config ntp and wait for time sync
  // default timeout is 10 seconds
  //DateTime.begin(/* timeout param */);
  DateTime.begin(ntpDelay);
  
  if (!DateTime.isTimeValid()) {
    Serial.println("Failed to get time from server.");
  }
  else{
    Serial.print("date and time (UTC):");
    Serial.println(DateTime.toUTCString());
  }
}


void GetExternalIP()
{
  ipExt = "1";
//  WiFiClient client;
//  if (!client.connect("api.ipify.org", 80)) {
//    Serial.println("Failed to connect with 'api.ipify.org' !");
//  }
//  else {
//    int timeout = millis() + 5000;
//    client.print("GET /?format=json HTTP/1.1\r\nHost: api.ipify.org\r\n\r\n");
//    while (client.available() == 0) {
//      if (timeout - millis() < 0) {
//        Serial.println(">>> Client Timeout !");
//        client.stop();
//        return;
//      }
//    }
//    int size;
//    while ((size = client.available()) > 0) {
//      uint8_t* msg = (uint8_t*)malloc(size);
//      size = client.read(msg, size);
//      Serial.write(msg, size);
//      Serial.println();
//
//      
////        //https://www.softwaretestinghelp.com/regex-in-cpp/#regex_search
////        //(?<=\{)(.*?)(?=\})
////        regex regexp("(?<=\{)(.*?)(?=\})"); 
////   
////        // flag type for determining the matching behavior (in this case on string objects)
////         smatch m; 
////        regex_search(payload, m, regexp); 
////        ipExt=m.as<String>();
//
//      
//      //terminal.flush();
//      //terminal.write(msg, size);
//      //terminal.flush();
//      free(msg);
//    }
//    
//    Serial.print("GetExternalIP: response:");
//    Serial.println(client.readString()); 
//  }
  HTTPClient http;
  String serverNameTest = "http://api.ipify.org";
  String serverPath = serverNameTest + "";
  
  // Your Domain name with URL path or IP address with path
  http.begin(serverPath.c_str());
  
  // Send HTTP GET request
  int httpResponseCode = http.GET();
  
  if (httpResponseCode>0) {
    Serial.print("GetExternalIP(): HTTP Response code: ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    Serial.println(payload);
    ipExt = payload;
    
  }
  else {
    Serial.print("GetExternalIP(): Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();
}

String sendHttpCommand(String _protocol, String _serverName, String _port, String _postGet, String _commands)
{// attention, _port doit avoir lui-meme les ":" (c'est a dire: _port := ":"+_port)
  sendHttpCommandResp = "notSet";
  Serial.println("sendHttpCommand()" );
  Serial.print("protocol: " );Serial.print(_protocol );Serial.print("," );
  Serial.print("serveur: " );Serial.print(_serverName );Serial.print("," );
  Serial.print("port: " );Serial.print(_port );Serial.print("," );
  Serial.print("postget: " );Serial.print(_postGet );Serial.print("," );
  Serial.print("commands: " );Serial.println(_commands );
  
  if(_protocol.compareTo("https") == 0 || _protocol.compareTo("http") == 0 )
  {
    
//    WiFiClient client;
////    if (!client.connect(_serverName, _port.toInt())) {
//    if (!client.connect(_serverName.c_str(), atoi(_port.c_str()) ) ) {
//      Serial.print("Failed to connect with ");
//      Serial.println(_serverName);
//    }
//    else {
//      int timeout = millis() + 5000;
//      client.print("GET /?format=json HTTP/1.1\r\nHost: "+_serverName+"\r\n\r\n");
//      while (client.available() == 0) {
//        Serial.print("testing client available:");
//        Serial.print(timeout);Serial.print(" < ");Serial.println(millis());
//        if (timeout - millis() < 0) {
//          Serial.println(">>> Client Timeout !");
//          client.stop();
//          return "couldNotConnect";
//        }
//        delay(100);
//      }
//      int size;
//      while ((size = client.available()) > 0) {
//        uint8_t* msg = (uint8_t*)malloc(size);
//        size = client.read(msg, size);
//        Serial.write(msg, size);
//        Serial.println();
//  
//        
//  //        //https://www.softwaretestinghelp.com/regex-in-cpp/#regex_search
//  //        //(?<=\{)(.*?)(?=\})
//  //        regex regexp("(?<=\{)(.*?)(?=\})"); 
//  //   
//  //        // flag type for determining the matching behavior (in this case on string objects)
//  //         smatch m; 
//  //        regex_search(payload, m, regexp); 
//  //        ipExt=m.as<String>();
//  
//        
//        //terminal.flush();
//        //terminal.write(msg, size);
//        //terminal.flush();
//        free(msg);
//      }
//      
//      Serial.println("sendHttpCommand: response:");
//      Serial.println(client.readString()); 
//    }
    HTTPClient http;
    //String serverNameTest = _protocol+"://"+ _serverName+":"+ _port; //"http://api.ipify.org";
    String serverNameTest = _protocol+"://"+ _serverName+ _port; //"http://api.ipify.org";
    String serverPath = serverNameTest + "?"+_commands;
    
    // Your Domain name with URL path or IP address with path
    http.begin(serverPath.c_str());
    
    // Send HTTP GET request
    int httpResponseCode;
    if(_postGet.compareTo("GET") == 0)
      httpResponseCode = http.GET();
    else
    {
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      String httpRequestData = _commands;
      httpResponseCode = http.POST(httpRequestData);
    }
    
    if (httpResponseCode>0) {
      Serial.print("sendHttpCommand(): HTTP Response code: ");
      Serial.println(httpResponseCode);
      //String payload = http.getString();
      sendHttpCommandResp = http.getString();
      Serial.println("sendHttpCommand(): HTTP Response payload: ");
      //Serial.println(payload);
      //sendHttpCommandResp = payload;
      Serial.println(sendHttpCommandResp);

    }
    else {
      Serial.print("sendHttpCommand(): Error code: ");
      Serial.println(httpResponseCode);
      sendHttpCommandResp = http.getString();
    }
    // Free resources
    http.end();
  
  }
  else{
    Serial.println("sendHttpCommand(): cannot send protocol - ");
    sendHttpCommandResp = "protocolNotKnonw";
    //http.end();
    //return sendHttpCommandResp;
  }
  return "ok";
}


String strs[20];
bool isHTTPS = true;
String splitBearer(String _authoriz, String _delimiter)
{
  String resultStr ="";
  int string_index = 0;
  if (_authoriz.startsWith("Bearer "))
  {
    string_index = 7;
    char* str;

       /* Calcul de la taille à allouer */
    size_t l = strlen( _authoriz.substring(_authoriz.indexOf(_delimiter)+1, _authoriz.length()).c_str() ) + 1;

    /* Allocation de la mémoire */
    str = (char *) malloc( l * sizeof(char) );
    assert( str != NULL );
    //Serial.println("[splitBearer] str est pourri, on tente de lui affecter qqch de la taille du hostname puis on le copie ");

    //*str = malloc(_hostname.length);
    strcpy(str, _authoriz.substring(_authoriz.indexOf(_delimiter)+1, _authoriz.length()).c_str());

    //resultStr = _authoriz.substr(_authoriz.indexOf(_delimiter), sizeOf(_authoriz));
    Serial.printf("[splitBearer] resultat: [%s]\n",str);
    if(str == NULL || str == "" )
    {
      return "";
    }
    return String(str);
  }
  else{
    Serial.printf("[splitBearer]: [%s] ne commence pas par 'Bearer ', retourne chaine vide\n",_authoriz);

    return "";
  }
}
void splitHostnamePort( String _hostname, String _delimiter)
{
  Serial.print("[splitHostnamePort] hostname = ");
  Serial.println(_hostname);


  //https://forum.arduino.cc/t/how-to-extract-server-domain-from-a-url-in-char/65140/4
  //combine avec https://www.tutorialspoint.com/find-if-a-string-begins-with-a-specific-set-of-characters-in-arduino
  int string_index = 0;  // assume there's no http://

  if (_hostname.startsWith("http://"))
  {
    string_index = 7;
    isHTTPS = false;
  }
  if (_hostname.startsWith("https://"))
  {
    string_index = 8;
    isHTTPS = true;
  }
  if (_hostname.startsWith("ws://"))
    string_index = 5;
  if (_hostname.startsWith("wss://"))
    string_index = 6;

    
  int StringCount = 0;
  char* str;
  if(str != NULL)
  {
      Serial.println("[splitHostnamePort] str est ok, on copie ");

    strcpy(str, _hostname.substring(string_index, _hostname.length()).c_str());
  }
  else{
    /* Calcul de la taille à allouer */
    size_t l = strlen( _hostname.substring(string_index, _hostname.length()).c_str() ) + 1;

    /* Allocation de la mémoire */
    str = (char *) malloc( l * sizeof(char) );
    assert( str != NULL );
          Serial.println("[splitHostnamePort] str est pourri, on tente de lui affecter qqch de la taille du hostname puis on le copie ");

    //*str = malloc(_hostname.length);
    strcpy(str, _hostname.substring(string_index, _hostname.length()).c_str());
  }

  Serial.print("[splitHostnamePort] hostname = ");
  Serial.println(_hostname);
  Serial.print("[splitHostnamePort] str = ");
  Serial.println(str);
  
  String strEnStr = String(str);
    while (strEnStr.length() > 0)
  {
    int index = strEnStr.indexOf(_delimiter);
    if (index == -1) // No space found
    {
      strs[StringCount++] = strEnStr;
      break;
    }
    else
    {
      strs[StringCount++] = strEnStr.substring(0, index);
      strEnStr = strEnStr.substring(index+1);
    }
  } 
   // Show the resulting substrings
  for (int i = 0; i < StringCount; i++)
  {
    Serial.print(i);
    Serial.print(": \"");
    Serial.print(strs[i]);
    Serial.println("\"");
  }

//  if(strs[0].compareTo("http")==0 || strs[0].compareTo("ws"))
//  { //plus besoin puisque les http, https ws wss sont filtre avant
//    socketHostname = strs[0]+":"+strs[1];
//    if(strs[2].length() >=0)
//      hostnamePort = strs[2].toInt();
//    else
//      hostnamePort = 80;
//  }
//  else
//  {
    socketHostname = strs[0];
    if(strs[1].length() >=0)
    {
      hostnamePort = strs[1].toInt();
      if(hostnamePort == 0)
      {
        if(isHTTPS)
          hostnamePort = 443;
        else
          hostnamePort = 80;
      }
    }
    else
    {
      if(isHTTPS)
        hostnamePort = 443;
      else
        hostnamePort = 80;
    }
//  }
  Serial.print("[SPLIT] socketHostname = ");
  Serial.println(socketHostname);
  Serial.print("[SPLIT] hostnamePort = ");
  Serial.println(hostnamePort);
  Serial.print("[SPLIT] hostname = ");
  Serial.println(_hostname);
  Serial.print("[SPLIT] str = ");
  Serial.println(str);
}





// Global variables for updatefromweb
int totalLength;       //total size of firmware
int currentLength = 0; //current size of written firmware

// Function to update firmware incrementally
// Buffer is declared to be 128 so chunks of 128 bytes
// from firmware is written to device until server closes
bool updateFirmware(uint8_t *data, size_t len){
  Update.write(data, len);
  currentLength += len;
  // Print dots while waiting for update to finish
  Serial.print('.');
  // if current length of written firmware is not equal to total firmware size, repeat
  if(currentLength != totalLength) return false;
  //Update.end(true);
  //Serial.printf("\nUpdate Success, Total Size: %u\nRebooting...\n", currentLength);
  Serial.print("\nUpdate Success, Total Size: ");Serial.println(currentLength);
  Serial.println("\nShould Reboot...\n");
  // Restart ESP32 to see changes 
  //ESP.restart();

  return true;
}
void fakeUpdateFirmware(uint8_t *data, size_t len){
  currentLength += len;
   Serial.print(String((char *) data));

  // Print dots while waiting for update to finish
  Serial.print('.');
  // if current length of written firmware is not equal to total firmware size, repeat
  if(currentLength != totalLength) return;
    Serial.printf("\nfakeUpdate Success, Total Size: %u\n Are you happy?...\n", currentLength);

}

int updateFromWeb(String _type, String _version)
{
      //https://www.teachmemicro.com/update-esp32-firmware-external-web-server/ 
      // et https://randomnerdtutorials.com/esp32-http-get-post-arduino/
      HTTPClient thisHttp;
      //String serverNameTest = hostname+"/api/updateFirmware?idPortier="+myId+"&type="+_type+"&version="+_version;
      String serverNameTest = updateHostname+_version+".bin";
      
      Serial.print("updateFromWeb: from serverNameTest = ");Serial.println(serverNameTest);

      String serverPath = serverNameTest + "";
      Serial.printf("updateFromWeb: from serverPath %s\n",serverPath);
      Serial.print("updateFromWeb: from serverPath println = ");Serial.println(serverPath);


      // Your Domain name with URL path or IP address with path
      thisHttp.begin(serverPath.c_str());
      
      // Send HTTP GET request
      int httpResponseCode = thisHttp.GET();
      
      if (httpResponseCode>0) {

        if(httpResponseCode == 200){
          // get length of document (is -1 when Server sends no Content-Length header)
          totalLength = thisHttp.getSize();
          // transfer to local variable
          int len = totalLength;
          // this is required to start firmware update process
          Update.begin(UPDATE_SIZE_UNKNOWN);
          Serial.printf("FW Size: %u\n",totalLength);
          // create buffer for read
          uint8_t buff[128] = { 0 };
          // get tcp stream
          WiFiClient * stream = thisHttp.getStreamPtr();
          // read all data from server
          Serial.println("Updating firmware...");
          if(trueFirmwareUpdate != NULL && !trueFirmwareUpdate.isEmpty() && trueFirmwareUpdate.compareTo("true") == 0 )
          { 
            Serial.println("reading file by TRUE UPDATE:");
          }else
          {
            Serial.println("reading file by FAKE update:");
          }

          while(thisHttp.connected() && (len > 0 || len == -1)) {
               // get available data size
               size_t size = stream->available();
               if(size) {
                  // read up to 128 byte
                  int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
                  //Serial.printf("updateFromWeb(): got file: %i\n",c);
                  // pass to function
                  if(trueFirmwareUpdate != NULL && !trueFirmwareUpdate.isEmpty() && trueFirmwareUpdate.compareTo("true") == 0 )
                  { 
                    updateFirmware(buff, c);
                  }else
                  {
                    fakeUpdateFirmware(buff,c);
                  }
                  if(len > 0) {
                     len -= c;
                  }
               }
               delay(1);
          }
          Serial.print("updateFromWeb: len           = "); Serial.println(len);
          Serial.print("updateFromWeb: currentLength = "); Serial.println(currentLength);
          Serial.print("updateFromWeb: totalLength   = "); Serial.println(totalLength);
          
          Update.end(true);
          currentLength = 0; 
          firmwareUpdated = true;
          Serial.println("should make the update for real");
          thisHttp.end();
          return 1;
        }
        else{
           Serial.println("Cannot download firmware file. Only HTTP response 200: OK is supported. Double check firmware location #defined in HOST.");
           thisHttp.end();
           return 0;
        }

        
        Serial.printf("updateFromWeb(%s): HTTP Response code: ",serverPath.c_str());
        Serial.println(httpResponseCode);
        String payload = thisHttp.getString();
        Serial.printf("updateFromWeb(%s): HTTP Response payload: ",serverPath.c_str());

        Serial.println(payload);
      }
      else {
        Serial.print("updateFromWeb(): Error code: ");
        Serial.println(httpResponseCode);
      }
      // Free resources
      thisHttp.end();
      return 0;

}


bool socketIsConnected( SocketIOclient _socketClient)
{ //https://github.com/Links2004/arduinoWebSockets/issues/393
  //https://stackoverflow.com/questions/10437584/socket-io-reconnect
  Serial.print("[socketIsConnected]: connected? ");
  Serial.println(_socketClient.isConnected());
  //Serial.println(_socketClient.socket.connected);
  //Serial.print("[socketIsConnected]: connecting? ");
  //Serial.println(_socketClient.socket.connecting);
  
  //return (_socketClient.socket.connected && _socketClient.socket.connecting);
  if(_socketClient.isConnected())
  {
    Serial.println("socketIsConnected: [isConnected] setting led ON for connected");
    ledState(SOCKET_CONNECTED);
  }
  else{
    Serial.println("socketIsConnected: [disconnected] setting led OFF for disconnected");
    isRegistred = false;
    ledState(SOCKET_DISCONNECTED);
  }
  return _socketClient.isConnected();

}
void checkSocketConnected(SocketIOclient _socketClient)
{
  unsigned long currentMillis = millis();
  if (currentMillis - previousSocketCheck >= 3*intervalSocketCheck) {
     //permet de remettre a 0 le compteur car cette condition indiquerait qu'il y avait une connection avant.
    Serial.println("[checkSocketConnected] reinitialisation de l'iteration iteSocketCheck à 0.");
    iteSocketCheck=0;
  }
    if (currentMillis - previousSocketCheck >= intervalSocketCheck) {
      previousSocketCheck = currentMillis;
      Serial.print("[checkSocketConnected] making check [");Serial.print(iteSocketCheck);Serial.print("/");Serial.print(maxIteSockDownBeforeAP);Serial.println("]");
//      Serial.println(iteSocketCheck);
//      if(_socketClient != NULL)
//      {
        if(!socketIsConnected( _socketClient))
        {
          iteSocketCheck++;
          if (iteSocketCheck > maxIteSockDownBeforeAP)
          {
            iteSocketCheck=0;
            makeAPOn = true;
            preferences.begin(prefNameSpace.c_str(), false);
              preferences.putBool("makeAPOn", makeAPOn);
            preferences.end();
            //A la barbare: on reboot alors qu'il serait preferable de stopper le wifi et lancer le serveur.
            Serial.print("[checkSocketConnected] restarting");
            ESP.restart();
            
          }
        }
//      }
//      else{
//        USE_SERIAL.println("[checkSocketConnected]  la socket est nulle");
//      }
//      //return false;
    }
    else
    {
      Serial.println("[checkSocketConnected] not checking.");
    }
    Serial.print("[checkSocketConnected] end [");Serial.print(iteSocketCheck);Serial.print("/");Serial.print(maxIteSockDownBeforeAP);Serial.println("]");

}

void socketIOEvent(socketIOmessageType_t type, uint8_t * payload, size_t length) {
  USE_SERIAL.printf("[IOc] enter with: "); USE_SERIAL.println(type);
    switch(type) {
        case sIOtype_DISCONNECT:
              Serial.print("[IOc] Disconnected running on core: ");
              Serial.println(xPortGetCoreID());
            USE_SERIAL.printf("[IOc] Disconnected!\n");
            isRegistred = false;

            if(withSocketFallToWifi)
              checkSocketConnected(socketIO);
//            if (currentMillis - previousMillis >= interval) {
//              Serial.println("[IOc] wifi: Failed to connect.");
//              return false;
//            }
//            previousMillisSocket = 0;
//            socketOffIntervalBeforWifi;

            Serial.println("socketIO type_DISCONNECTED: setting led OFF for disconnecte");
            ledState(SOCKET_DISCONNECTED);

            break;
//        case sIOtype_CONNECT_ERROR:
//            Serial.println("socketIO sIOType_CONNECT_ERROR");
//            checkSocketConnected(socketIO);
//            break;
        case sIOtype_CONNECT:
            //USE_SERIAL.printf("[IOc] Connected to url: %s, with %s\n", payload, socketIO.toString());
            USE_SERIAL.printf("[IOc] Connected to url: %s\n", payload);
            //Serial.println("socketIO: Type_CONNECT: setting led ON for connected");
            //ledState(SOCKET_CONNECTED);
            Serial.print("[IOc] Connected running on core: ");
            Serial.println(xPortGetCoreID());

            // join default namespace (no auto join in Socket.IO V3)
            socketIO.send(sIOtype_CONNECT, "/");


            //fait le login du portier
            now = millis();

            if(now - messageTimestamp > 1500) {
                messageTimestamp = now;
        
                // creat JSON message for Socket.IO (event)
                DynamicJsonDocument doc(1024);
                JsonArray array = doc.to<JsonArray>();
        
                // add evnet name
                // Hint: socket.on('event_name', ....
                array.add("portierLogin");
        
                // add payload (parameters) for the event
                JsonObject param1 = array.createNestedObject();
                //param1["req"] = "login";
                param1["id"] =  DateTime.now();
                param1["login"] = login; //"portier2-AAAA";//(uint32_t) now;
                param1["pwd"] = pwd; //"J35u1sUnP0rT1e4-AAAA";
                param1["version"] = version;
                param1["ethOn"] = String(useEth);
                param1["macEth"] = ETH.macAddress();
                param1["macWifi"] = ESP.getEfuseMac();
//                param1["idSocketCaller"] = "idclaller1";
//                param1["idPorte"] = "1";
//                param1["idPortier"] = "7445";
//                param1["state"] = "1";
                //param1["date"] = (uint32_t) now;
                //param1["date"] = (uint32_t) messageTimestamp;
                if (!DateTime.isTimeValid()) {
                  Serial.println("Failed to get time from server, retry.");
                  DateTime.begin();
                  delay(250);
                  if(!DateTime.isTimeValid())
                  {
                    param1["date"] = DateTime.now();
                  }else{
                    param1["date"] = (uint32_t) messageTimestamp;
                  }
                } else {
                  param1["date"] = DateTime.now();
                }
        
                // JSON to String (serializion)
                String output;
                serializeJson(doc, output);

                // Send event
                delay(250);
                socketIO.sendEVENT(output);

// essaies pour pouvoir faire un 2e appel si je n'ai pas de ack                 
//              unsigned long currentMillis;
//              bool tryAgain = true;
//          
//                if(!isRegistred && tryAgain)
//                {
//                  iteSocketCheck++;
//                  if (iteSocketCheck > maxIteSockDownBeforeAP)
//                  {
//                    tryAgain = false;
//                    iteSocketCheck = 0;
//                    Serial.print("[socketIOEvent] Socket Connected -> restarting");
//                    ESP.restart();
//                    
//                  }
//                  socketIOEvent(type, payload, length);
//                }

//                while(!isRegistred && tryAgain)  
////                while(!isRegistred && socketIsConnected(socketIO) && tryAgain)  
////                if(!isRegistred && tryAgain)  
//
//                {
//                      socketIO.sendEVENT(output);
//
//
//                  
//                
//                  Serial.print("[socketIOEvent] Socket Connected -> call [");Serial.print(iteSocketCheck);Serial.print("/");Serial.print(maxIteSockDownBeforeAP);Serial.println("]");
//
//                  ledState(SOCKET_DISCONNECTED); //pas top mais socketIsConnected fait aussi un changement d'etat => a revoir
//
//                  //fait un envoi toutes les [intervalSocketCheck] secondes
//                  currentMillis = millis();
//
//                  if (currentMillis - previousSocketCheck >= intervalSocketCheck) { // parial copy of checkSocketConnected
//                    previousSocketCheck = currentMillis;
//                    Serial.print("[socketIOEvent] Socket Connected -> making registration check [");Serial.print(iteSocketCheck);Serial.print("/");Serial.print(maxIteSockDownBeforeAP);Serial.println("]");
//                    
//                      iteSocketCheck++;
//                      if (iteSocketCheck > maxIteSockDownBeforeAP)
//                      {
//                        tryAgain = false;
//                        // Send event
//                        
//                      }
//                      Serial.print("[socketIOEvent] Socket Connected -> sending event [");Serial.print(iteSocketCheck);Serial.print("/");Serial.print(maxIteSockDownBeforeAP);Serial.println("]");
//                      socketIO.sendEVENT(output);
//                      Serial.print("[socketIOEvent] Socket Connected -> event sent [");Serial.print(iteSocketCheck);Serial.print("/");Serial.print(maxIteSockDownBeforeAP);Serial.println("]");
//                      
//                  }
//                  Serial.print("[socketIOEvent] Socket Connected -> Sleeping for 1s [");Serial.print(iteSocketCheck);Serial.print("/");Serial.print(maxIteSockDownBeforeAP);Serial.println("]");
//                  //sleep(1000);
//                  delay(500);
//                }
//                if(!isRegistred && tryAgain)  
//                {
//                  Serial.print("[socketIOEvent] Socket Connected -> registering 2e essai ");
//
//                  socketIO.sendEVENT(output);
//                  //sleep(1000);
//                  delay(500);
//                }
//                if(!isRegistred)
//                {
//                  Serial.print("[socketIOEvent] Socket Connected -> restarting");
//
//                  ESP.restart();
//                }
//        
                // Print JSON for debugging
                USE_SERIAL.println(output);
                //notifyConnected(NULL);
                runTaskNotifyConnect();


                if (firmwareUpdated)
                {
                  Serial.println("confirmer l'update du firmware");
                        //DynamicJsonDocument docAns(1024);
                        //  JsonArray array = docAns.to<JsonArray>();
                  
                          // add evnet name
                          // Hint: socket.on('event_name', ....
                          array.clear();
                          array.add("ack");
                  
                          // add payload (parameters) for the event
                          //JsonObject param1 = array.createNestedObject();
                          param1 = array.createNestedObject();
                          //param1["req"] = "login";
                          param1["id"] = updateFirmware_idComment;//doc[1]["id"];
                          param1["req"] = "updateFirmware";
                          param1["todo"] = "firmware";
                          param1["what"] = "updateFirmware";
                          param1["type"] = updateFirmware_type; //doc[1]["type"];
                          param1["version"] = version;
                          param1["idSocketCaller"] = updateFirmware_idSocketCaller;//doc[1]["idSocketCaller"];
                          param1["idCaller"] = updateFirmware_idCaller ; //doc[1]["idCaller"];
                          //pour tricher en attendant
                          myId = doc[1]["idPortier"].as<String>();
                          param1["idPortier"] = myId;
                          param1["ents"] = updateFirmware_ents;//doc[1]["ents"];
                          if(updateFirmware_oldVers.compareTo(version) == 0 || updateFirmware_oldVers.compareTo("") == 0)
                          {
                            param1["state"] = "0";
                            param1["text"] = "update FAILED :version: "+version+", old version: "+updateFirmware_oldVers;
                          }
                          else
                          {
                            param1["state"] = "4";
                            param1["text"] = "update done with new version: "+version;
                          }
                          //param1["date"] = (uint32_t) messageTimestamp;
                          param1["date"] = DateTime.now();

                  
                          // JSON to String (serializion)
                          String output;
                          serializeJson(doc, output);
                  
                          // Send event
                          socketIO.sendEVENT(output);
                  
                          // Print JSON for debugging
                          Serial.println("socket update firmware: done, notifying on connect, sending:");
                          USE_SERIAL.println(output);
                          firmwareUpdated = false;
                          preferences.begin( prefNameSpace.c_str(), false);
                            preferences.putBool("firmwareUpdated", firmwareUpdated);
                          preferences.end();
                }
            }
            
            break;
        case sIOtype_EVENT:
        {
            char * sptr = NULL;
            int id = strtol((char *)payload, &sptr, 10);
            USE_SERIAL.printf("[IOc] get event: %s id: %d\n", payload, id);
            if(id) {
                payload = (uint8_t *)sptr;
            }

            DynamicJsonDocument doc(1024);
            DeserializationError error = deserializeJson(doc, payload, length);
            if(error) {
                USE_SERIAL.print(F("deserializeJson() failed: "));
                USE_SERIAL.println(error.c_str());
                return;
            }

            String eventName = doc[0];
            String doc1Test = doc[1];
            USE_SERIAL.printf("[IOc] event name: %s\n", eventName.c_str());
            //USE_SERIAL.printf("[IOc] doc[1] en c_str(): %s\n", doc1 ["req"]);


                if( eventName.compareTo("message") == 0)
                {

                  String doc1 = doc[1][0];
                  String doc2 = doc[1][1];
                  String doc3 = doc[1][2];
                  USE_SERIAL.printf("[IOc] doc: %s \n", doc);
                  USE_SERIAL.printf("[IOc] doc[1] %s \n", doc1);
                  USE_SERIAL.printf("[IOc] doc[2] %s \n", doc2);
                  USE_SERIAL.printf("[IOc] doc[3] %s \n", doc3);
                }
                else if(eventName.compareTo("portier") == 0)
                {
                  String doc1 = doc[1]["req"];
                  String doc2 = doc[1]["todo"];
                  int idPorte = doc[1]["idPorteChezPortier"].as<int>();
                  int triggeredDelay = doc[1]["triggeredDelay"].as<int>();

                  USE_SERIAL.printf("[IOc] portier doc: %s \n", doc);
                  USE_SERIAL.printf("[IOc] portier doc[1] %s \n", doc1);
                  USE_SERIAL.printf("[IOc] portier doc[2] %s \n", doc2);
                  USE_SERIAL.printf("[IOc] portier doc[3] %i \n", idPorte);

                   const char* jsonHostname = doc[1]["todo"];
                    if (jsonHostname)
                    {
                      Serial.print("jsonName todo existe: ");
                      Serial.println(jsonHostname);
                    }
                    else{
                      Serial.print("req not found ");
                    }
                    Serial.print("hostname = ");
                    Serial.println(hostname);
                    const char* jsonreq = doc[1]["req"];
                    //const char* jsonreq = jsonMess["req"];
                    if (jsonreq)
                    {   
                       Serial.print("req Trouve: ");
                       Serial.println(jsonreq);
                    }
                    
                    if((String(jsonreq)).compareTo("door") == 0)
                    {//on envoit l'ouverture de porte
                      if (triggeredDelay > delayOn)
                        openDoor(idPorte, triggeredDelay);
                      else
                        openDoor(idPorte);
                        
                        delay(250);
    
                        //Answering
       // creat JSON message for Socket.IO (event)
                      DynamicJsonDocument docAns(1024);
                      JsonArray array = docAns.to<JsonArray>();
              
                      // add evnet name
                      // Hint: socket.on('event_name', ....
                      array.add("message");
              
                      // add payload (parameters) for the event
                      JsonObject param1 = array.createNestedObject();
                      //param1["req"] = "login";
                      param1["id"] =  doc[1]["id"];
                      param1["req"] = doc[1]["req"];
                      param1["todo"] = "openRes";
                      param1["pwd"] = pwd; //"J35u1sUnP0rT1e4-AAAA";
                      param1["idSocketCaller"] = doc[1]["idSocketCaller"];
                      param1["idCaller"] = doc[1]["idCaller"];
                      param1["idPorte"] = doc[1]["idPorte"];
                      param1["gpioUsed"] = doors_to_relay[idPorte];
                        //pour tricher en attendant
                        myId = doc[1]["idPortier"].as<String>();
                      param1["idPortier"] = myId;
                      param1["ents"] = doc[1]["ents"];
                      param1["state"] = "4";
                      //param1["date"] = (uint32_t) now;
                      param1["date"] = DateTime.now();
              
                      // JSON to String (serializion)
                      String output;
                      serializeJson(docAns, output);
              
                      // Send event
                      socketIO.sendEVENT(output);
              
                      // Print JSON for debugging
                      USE_SERIAL.println("sending Acklike for opening:");
                      USE_SERIAL.println(output);
    
                        //-- answering
                    }
                    else if((String(jsonreq)).compareTo("command") == 0)
                    {//on envoit une requete reseau
                        Serial.println("[portier]: devrait envoyer une commande html");

//                        sendHttpCommandResp = sendHttpCommand(new String(doc[1]["protcol"]), new String(doc[1]["server"]), new String(doc[1]["port"]), new String(doc[1]["getpost"]), new String(doc[1]["fullCommand"]) );
                        sendHttpCommand(doc[1]["protcol"], doc[1]["server"],doc[1]["port"], doc[1]["getpost"], doc[1]["fullCommand"] );
                        Serial.println("fini");

                        delay(250);

                        Serial.println("answering");

                        //Answering
       // creat JSON message for Socket.IO (event)
                      DynamicJsonDocument docAns(1024);
                      JsonArray array = docAns.to<JsonArray>();
              Serial.println("answering json cree");
                      // add evnet name
                      // Hint: socket.on('event_name', ....
                      array.add("message");
              Serial.println("answering message ajoute");
                      // add payload (parameters) for the event
                      JsonObject param1 = array.createNestedObject();
                Serial.println("answering jsonObject ok");
                      //param1["req"] = "login";
                      param1["id"] =  doc[1]["id"];
                      param1["req"] = doc[1]["req"];
                      param1["todo"] = "openRes";
                      param1["pwd"] = pwd; //"J35u1sUnP0rT1e4-AAAA";
                      param1["idSocketCaller"] = doc[1]["idSocketCaller"];
                      param1["idCaller"] = doc[1]["idCaller"];
                      param1["idPorte"] = doc[1]["idPorte"];
                      param1["answer"] = sendHttpCommandResp.c_str();
                        //pour tricher en attendant
                        myId = doc[1]["idPortier"].as<String>();
                      param1["idPortier"] = myId;
                      param1["ents"] = doc[1]["ents"];
                      param1["state"] = "4";
                      //param1["date"] = (uint32_t) now;
                      param1["date"] = DateTime.now();

              Serial.println("answering serialisation");
                      // JSON to String (serializion)
                      String output;
                      serializeJson(docAns, output);
              
                      // Send event
                      socketIO.sendEVENT(output);
              
                      // Print JSON for debugging
                      USE_SERIAL.println("sending Acklike for opening:");
                      USE_SERIAL.println(output);
    
                        //-- answering
                    
                    }
                    else{
                      //non door et non command
                      //Answering
                      // creat JSON message for Socket.IO (event)
                      DynamicJsonDocument docAns(1024);
                      JsonArray array = docAns.to<JsonArray>();
              
                      // add evnet name
                      // Hint: socket.on('event_name', ....
                      array.add("message");
              
                      // add payload (parameters) for the event
                      JsonObject param1 = array.createNestedObject();
                      //param1["req"] = "login";
                      param1["id"] =  doc[1]["id"];
                      param1["req"] = doc[1]["req"];
                      param1["todo"] = "openRes";
                      param1["pwd"] = pwd; //"J35u1sUnP0rT1e4-AAAA";
                      param1["idSocketCaller"] = doc[1]["idSocketCaller"];
                      param1["idCaller"] = doc[1]["idCaller"];
                      param1["idPorte"] = doc[1]["idPorte"];
                      param1["answer"] = "unknownCommand";
                        //pour tricher en attendant
                        myId = doc[1]["idPortier"].as<String>();
                      param1["idPortier"] = myId;
                      param1["ents"] = doc[1]["ents"];
                      param1["state"] = "0";
                      //param1["date"] = (uint32_t) now;
                      param1["date"] = DateTime.now();
              
                      // JSON to String (serializion)
                      String output;
                      serializeJson(docAns, output);
              
                      // Send event
                      socketIO.sendEVENT(output);
              
                      // Print JSON for debugging
                      USE_SERIAL.println("sending Acklike for opening:");
                      USE_SERIAL.println(output);
    
                        //-- answering
                    }
                }
                else if(eventName.compareTo("getMyPassword") == 0)
                {
                  DynamicJsonDocument docAns(1024);
                  JsonArray array = docAns.to<JsonArray>();
                  String output;
                  serializeJson(docAns, output);
                  array.add("ack");
          
                  // add payload (parameters) for the event
                  JsonObject param1 = array.createNestedObject();
                  //param1["req"] = "login";
                  param1["id"] =  doc[1]["id"];
                  param1["req"] = doc[1]["req"];
                  param1["what"] =  "revealingPassword";
                  param1["value"] = editingPassword;
                  param1["idSocketCaller"] = doc[1]["idSocketCaller"];
                  param1["idCaller"] = doc[1]["idCaller"];
                  param1["idPortier"] = myId;
                  param1["ents"] = doc[1]["ents"];
                  param1["date"] = DateTime.now();
                  
                  serializeJson(docAns, output);
                  // Send event
                  socketIO.sendEVENT(output);
                  
                            // Print JSON for debugging
                  USE_SERIAL.println("sending getPassword to reveal:");
                  USE_SERIAL.println(output);
                }
                else if(eventName.compareTo("getExtIp") == 0)
                {
                Serial.println("Requesting getting ext ipAddress");
                      GetExternalIP();
                  
                  DynamicJsonDocument docAns(1024);
                  JsonArray array = docAns.to<JsonArray>();
                  String output;
                  serializeJson(docAns, output);
                  array.add("ack");
          
                  // add payload (parameters) for the event
                  JsonObject param1 = array.createNestedObject();
                  //param1["req"] = "login";
                  param1["id"] =  doc[1]["id"];
                  param1["req"] = doc[1]["req"];
                  param1["what"] =  "getExtIp";
                  param1["value"] = ipExt;
                  param1["idSocketCaller"] = doc[1]["idSocketCaller"];
                  param1["idCaller"] = doc[1]["idCaller"];
                  param1["idPortier"] = myId;
                  param1["ents"] = doc[1]["ents"];
                  param1["date"] = DateTime.now();
                  
                  serializeJson(docAns, output);
                  // Send event
                  socketIO.sendEVENT(output);
                  
                            // Print JSON for debugging
                  USE_SERIAL.println("sending getExtIp:");
                  USE_SERIAL.println(output);
                }
                else if(eventName.compareTo("setDate") == 0)
                {
                Serial.println("Requesting getting ext ipAddress");
                      GetExternalIP();
                  
                  DynamicJsonDocument docAns(1024);
                  JsonArray array = docAns.to<JsonArray>();
                  String output;
                  serializeJson(docAns, output);
                  array.add("ack");
          
                  // add payload (parameters) for the event
                  JsonObject param1 = array.createNestedObject();
                  //param1["req"] = "login";
                  param1["id"] =  doc[1]["id"];
                  param1["req"] = doc[1]["req"];
                  param1["what"] =  "seDate";
                  param1["value"] = doc[1]["req"];
                  param1["ipExt"] = ipExt;
                  param1["idSocketCaller"] = doc[1]["idSocketCaller"];
                  param1["idCaller"] = doc[1]["idCaller"];
                  param1["idPortier"] = myId;
                  param1["ents"] = doc[1]["ents"];
                  param1["date"] = DateTime.now();
                  
                  serializeJson(docAns, output);
                  // Send event
                  socketIO.sendEVENT(output);
                  
                            // Print JSON for debugging
                  USE_SERIAL.println("sending setDate:");
                  USE_SERIAL.println(output);
                }
                else if(eventName.compareTo("update") == 0)
                { 
                  
                  String doc1 = doc[1]["req"];
                  String doc2 = doc[1]["todo"];
                  if(doc2.compareTo("relayInversion") == 0)
                  {
                    inverted =  doc[1]["inverted"].as<bool>();
                    preferences.begin(prefNameSpace.c_str(), false);
                      preferences.putBool("inverted",inverted);
                    preferences.end();
                  
                    pinInit();
                        //Answering
     // creat JSON message for Socket.IO (event)
                    DynamicJsonDocument docAns(1024);
                    JsonArray array = docAns.to<JsonArray>();
            
                    // add evnet name
                    // Hint: socket.on('event_name', ....
                    array.add("ack");
            
                    // add payload (parameters) for the event
                    JsonObject param1 = array.createNestedObject();
                    //param1["req"] = "login";
                    param1["id"] =  doc[1]["id"];
                    param1["req"] = doc[1]["req"];
                    param1["todo"] = "inversionDone";
                    param1["what"] = "relayInversion";
    
                    param1["pwd"] = pwd; //"J35u1sUnP0rT1e4-AAAA";
                    param1["idSocketCaller"] = doc[1]["idSocketCaller"];
                    param1["idCaller"] = doc[1]["idCaller"];
                    //pour tricher en attendant
                    myId = doc[1]["idPortier"].as<String>();
                    param1["idPortier"] = myId;
                    param1["ents"] = doc[1]["ents"];
                    param1["state"] = "4";
                    param1["inverted"] = inverted;
                    //param1["date"] = (uint32_t) now;
                    param1["date"] = DateTime.now();
            
                    // JSON to String (serializion)
                    String output;
                    serializeJson(docAns, output);
            
                    // Send event
                    socketIO.sendEVENT(output);
            
                    // Print JSON for debugging
                    USE_SERIAL.println(output);
                  }
                  else if(doc2.compareTo("gpioLed") == 0)
                  {
                    ledcDetachPin(doors_to_relay[gpioLed]);
                    //ledcAttachPin(doors_to_relay[gpioLed], ledChannel);
                    gpioLed =  doc[1]["gpioLed"].as<int>();
                    preferences.begin(prefNameSpace.c_str(), false);
                      preferences.putInt("gpioLed",gpioLed);
                      preferences.putString("gpioLedStr", (new String(gpioLed))->c_str());
                      gpioLed = preferences.getInt("gpioLed",999);
                      
                      USE_SERIAL.print("changement GPIOLED pour :");
                      USE_SERIAL.println(gpioLed);

                    preferences.end();

                    //idealement, il faudrait un try sur l'existance de gepioLed
                    ledInit();
                    //ledcAttachPin(doors_to_relay[gpioLed], ledChannel);
                    
                    //pinInit();
                        //Answering
     // creat JSON message for Socket.IO (event)
                    DynamicJsonDocument docAns(1024);
                    JsonArray array = docAns.to<JsonArray>();
            
                    // add evnet name
                    // Hint: socket.on('event_name', ....
                    array.add("ack");
            
                    // add payload (parameters) for the event
                    JsonObject param1 = array.createNestedObject();
                    //param1["req"] = "login";
                    param1["id"] =  doc[1]["id"];
                    param1["req"] = doc[1]["req"];
                    param1["todo"] = "gpioLedDone";
                    param1["what"] = "gpioLed"; 
    
                    param1["pwd"] = pwd; //"J35u1sUnP0rT1e4-AAAA";
                    param1["idSocketCaller"] = doc[1]["idSocketCaller"];
                    param1["idCaller"] = doc[1]["idCaller"];
                    //pour tricher en attendant
                    myId = doc[1]["idPortier"].as<String>();
                    param1["idPortier"] = myId;
                    param1["ents"] = doc[1]["ents"];
                    param1["state"] = "4";
                    param1["gpioLed"] = gpioLed;
                    //param1["date"] = (uint32_t) now;
                    param1["date"] = DateTime.now();
            
                    // JSON to String (serializion)
                    String output;
                    serializeJson(docAns, output);
            
                    // Send event
                    socketIO.sendEVENT(output);
            
                    // Print JSON for debugging
                    USE_SERIAL.println(output);
                  }
                  else if(doc2.compareTo("firmware") == 0)
                  {
                    String EspType = doc[1]["type"];
                    String wishedVersion = doc[1]["version"];
                    String portierPassword = doc[1]["editingPwd"];
                    trueFirmwareUpdate = doc[1]["trueupdate"].as<String>();
                    Serial.print("portierpasse != NULL?");Serial.println(String(portierPassword != NULL));
                    Serial.print("!portierPassword.isEmpty()?");Serial.println(String(!portierPassword.isEmpty()));
                    Serial.printf("portierPassword[%s].compareTo(editingPassword[%s]) == 0?",portierPassword.c_str(), editingPassword.c_str());Serial.println(String(portierPassword.compareTo(editingPassword) == 0));
                    Serial.print("trueFirmwareUpdate != NULL?");Serial.print(String(trueFirmwareUpdate != NULL));Serial.printf("trueFirmwareUpdate [%s]",trueFirmwareUpdate.c_str());
                    Serial.println();

                    if(portierPassword != NULL && !portierPassword.isEmpty() && portierPassword.compareTo(editingPassword) == 0 )
                    {
                      Serial.println("faire les verifications des Strings + changer myId pour qu'il soit celui de ce portier");


                      DynamicJsonDocument docAns(1024);
                          JsonArray array = docAns.to<JsonArray>();
                  
                          // add evnet name
                          // Hint: socket.on('event_name', ....
                          array.add("ack");
                  
                          // add payload (parameters) for the event
                          JsonObject param1 = array.createNestedObject();
                          //param1["req"] = "login";
                          param1["id"] =  doc[1]["id"];
                          param1["req"] = doc[1]["req"];
                          param1["todo"] = "firmware";
                          param1["what"] = "updateFirmware";
                          param1["type"] = EspType;
                          param1["version"] = wishedVersion;
                          param1["idSocketCaller"] = doc[1]["idSocketCaller"];
                          param1["idCaller"] = doc[1]["idCaller"];
                          //pour tricher en attendant
                          myId = doc[1]["idPortier"].as<String>();
                          param1["idPortier"] = myId;
                          param1["ents"] = doc[1]["ents"];
                          param1["state"] = "2";
                          param1["text"] = "Got the info, getting ready to update";
                          param1["date"] = DateTime.now();
                          if(trueFirmwareUpdate != NULL && !trueFirmwareUpdate.isEmpty())
                          {
                            trueFirmwareUpdate = doc[1]["trueupdate"].as<String>();
                          }else
                          {
                            trueFirmwareUpdate = "false";
                          }
                          // JSON to String (serializion)
                          String output;
                          serializeJson(docAns, output);
                  
                          // Send event
                          socketIO.sendEVENT(output);
                  
                          // Print JSON for debugging
                          Serial.println("socket update firmware: got the info, sending:");
                          USE_SERIAL.println(output);
                      
                      myId = doc[1]["idPortier"].as<String>();
                      if(updateFromWeb(EspType, wishedVersion) == 1)
                      {//pas besoin, il fait un restart avant.
                        Serial.println("faire l'envoi de la socket pour informer que c'est bon");
                        array.clear();
                          array.add("ack");
                  
                          // add payload (parameters) for the event
                          //JsonObject param1 = array.createNestedObject();
                          param1 = array.createNestedObject();
                          //param1["req"] = "login";
                          param1["id"] =  doc[1]["id"];
                          param1["req"] = doc[1]["req"];
                          param1["todo"] = "firmware";
                          param1["what"] = "updateFirmware";
                          param1["type"] = doc[1]["type"];
                          param1["version"] = doc[1]["version"];
                          param1["idSocketCaller"] = doc[1]["idSocketCaller"];
                          param1["idCaller"] = doc[1]["idCaller"];
                          //pour tricher en attendant
                          myId = doc[1]["idPortier"].as<String>();
                          param1["idPortier"] = myId;
                          param1["ents"] = doc[1]["ents"];
                          param1["state"] = "3";
                          param1["text"] = "update file loading done, restarting the doorkeeper";
                          param1["date"] = DateTime.now();
                  
                          // JSON to String (serializion)
                          String output;
                          serializeJson(docAns, output);
                  
                          // Send event
                          socketIO.sendEVENT(output);
                  
                          // Print JSON for debugging
                          Serial.println("socket update firmware: ok, sending:");
                          USE_SERIAL.println(output);

                          //sauvegarde des infos de la socket pour l'envoyer au redemarrage

                           preferences.begin( prefNameSpace.c_str(), false);
 
                            preferences.putBool("firmwareUpdated", firmwareUpdated);
                            
                            Serial.print("preference. put String - updateFirmware_idComment = ");Serial.println(doc[1]["id"].as<int>());
                            preferences.putInt("upFirm_idCom", doc[1]["id"].as<int>());
                            //preferences.putString("updateFirmware_idComment", doc[1]["id"]);
                            Serial.print("preference. put String - updateFirmware_idSocketCaller = ");Serial.println(doc[1]["idSocketCaller"].as<char*>());
                            preferences.putString("upFirm_idSocket", doc[1]["idSocketCaller"].as<char*>()) ;
                            Serial.print("preference. put String - updateFirmware_idCaller = ");Serial.println(doc[1]["idCaller"].as<char*>());
                            preferences.putString("upFirm_idCaller", doc[1]["idCaller"].as<char*>());
                            
                            Serial.print("preference. put String - updateFirmware_type = ");Serial.println(EspType);
                            preferences.putString("upFirm_type", EspType);   
                            
                            Serial.print("preference. put String - updateFirmware_ents = ");Serial.println(doc[1]["ents"].as<char*>());
                            preferences.putString("upFirm_ents", doc[1]["ents"].as<char*>());
                            
                            Serial.print("preference. put String - oldVersion = ");Serial.println(version);
                            preferences.putString("upFirm_oldVers", version);
                           //preferences.end();
                           //preferences.begin( prefNameSpace.c_str(), false);

                            updateFirmware_idComment = preferences.getInt("upFirm_idCom",0);
                            Serial.print("firmwareupdate => updateFirmware_idComment = ");
                            Serial.println(updateFirmware_idComment);
                            updateFirmware_idSocketCaller  = preferences.getString("upFirm_idSocket","pasbonidSocketCaller");
                            Serial.print("firmwareupdate => updateFirmware_idSocketCaller = ");
                            Serial.println(updateFirmware_idSocketCaller);
                           preferences.end();
                           ESP.restart();
                        
                      }else
                      {
                        Serial.println("faire l'envoi de la socket pour informer que c'est raté");
                        //DynamicJsonDocument docAns(1024);
                        //  JsonArray array = docAns.to<JsonArray>();
                  
                          // add evnet name
                          // Hint: socket.on('event_name', ....
                          array.clear();
                          array.add("ack");
                  
                          // add payload (parameters) for the event
                          //JsonObject param1 = array.createNestedObject();
                          param1 = array.createNestedObject();
                          //param1["req"] = "login";
                          param1["id"] =  doc[1]["id"];
                          param1["req"] = doc[1]["req"];
                          param1["todo"] = "firmware";
                          param1["what"] = "updateFirmware";
                          param1["type"] = doc[1]["type"];
                          param1["version"] = doc[1]["version"];
                          param1["idSocketCaller"] = doc[1]["idSocketCaller"];
                          param1["idCaller"] = doc[1]["idCaller"];
                          //pour tricher en attendant
                          myId = doc[1]["idPortier"].as<String>();
                          param1["idPortier"] = myId;
                          param1["ents"] = doc[1]["ents"];
                          param1["state"] = "0";
                          param1["text"] = "something went wrong when updating";
                          param1["date"] = DateTime.now();
                  
                          // JSON to String (serializion)
                          String output;
                          serializeJson(docAns, output);
                  
                          // Send event
                          socketIO.sendEVENT(output);
                  
                          // Print JSON for debugging
                          Serial.println("socket update firmware: error, sending:");
                          USE_SERIAL.println(output);
                      }
                      
                    }
                    else
                    {
                      Serial.println("bad password, return socket to inform this.");
                      DynamicJsonDocument docAns(1024);
                    JsonArray array = docAns.to<JsonArray>();
            
                    // add evnet name
                    // Hint: socket.on('event_name', ....
                    array.add("ack");
            
                    // add payload (parameters) for the event
                    JsonObject param1 = array.createNestedObject();
                    //param1["req"] = "login";
                    param1["id"] =  doc[1]["id"];
                    param1["req"] = doc[1]["req"];
                    param1["todo"] = "firmware";
                    param1["what"] = "updateFirmware";
                    param1["type"] = doc[1]["type"];
                    param1["version"] = doc[1]["version"];
                    param1["idSocketCaller"] = doc[1]["idSocketCaller"];
                    param1["idCaller"] = doc[1]["idCaller"];
                    //pour tricher en attendant
                    myId = doc[1]["idPortier"].as<String>();
                    param1["idPortier"] = myId;
                    param1["ents"] = doc[1]["ents"];
                    param1["state"] = "0";
                    param1["text"] = "bad password";
                    param1["date"] = DateTime.now();
            
                    // JSON to String (serializion)
                    String output;
                    serializeJson(docAns, output);
            
                    // Send event
                    socketIO.sendEVENT(output);
            
                    // Print JSON for debugging
                    Serial.println("socket update firmware: error, sending:");
                    USE_SERIAL.println(output);
                    }
                  }
                }
                else if(eventName.compareTo("maintenance") == 0)
                { 
                  
                  String doc1 = doc[1]["req"];
                  String doc2 = doc[1]["todo"];
                  if(doc2=="putMaintenanceOn")
                  {
                    makeAPOn = true;
                    maintenanceOn = true;
                    preferences.begin(prefNameSpace.c_str(), false);
                      preferences.putBool("makeAPOn", makeAPOn);
                      preferences.putBool("maintenanceOn", maintenanceOn);
                    preferences.end();
                    
                    // creat JSON message for Socket.IO (event)
                    DynamicJsonDocument docAns(1024);
                    JsonArray array = docAns.to<JsonArray>();
            
                    // add evnet name
                    // Hint: socket.on('event_name', ....
                    array.add("ack");
            
                    // add payload (parameters) for the event
                    JsonObject param1 = array.createNestedObject();
                    //param1["req"] = "login";
                    param1["id"] =  doc[1]["id"];
                    param1["req"] = doc[1]["req"];
                    param1["todo"] = "putMaintenanceOn";
                    param1["what"] = "maintenance";
                    param1["idSocketCaller"] = doc[1]["idSocketCaller"];
                    param1["idCaller"] = doc[1]["idCaller"];
                    //pour tricher en attendant
                    myId = doc[1]["idPortier"].as<String>();
                    param1["idPortier"] = myId;
                    param1["ents"] = doc[1]["ents"];
                    param1["state"] = "1";
                    param1["maintenanceOn"] = maintenanceOn;
                    param1["delay"] = String(XminDelay);
                    //param1["date"] = (uint32_t) now;
                    param1["date"] = DateTime.now();
            
                    // JSON to String (serializion)
                    String output;
                    serializeJson(docAns, output);
            
                    // Send event
                    socketIO.sendEVENT(output);
            
                    // Print JSON for debugging
                    USE_SERIAL.println(output);
                    delay(300);
                    ESP.restart();
                  }
                }
                else if(eventName.compareTo("gpio") == 0)
                {
                  String doc1 = doc[1]["req"];
                  String doc2 = doc[1]["todo"];
                  int idPorte = doc[1]["idPorteChezPortier"].as<int>();
                  int idGpio = doc[1]["gpio"].as<int>();

                  int triggeredDelay = doc[1]["triggeredDelay"].as<int>();

                  USE_SERIAL.printf("[IOc] gpio portier doc: %s \n", doc);
                  USE_SERIAL.printf("[IOc] gpio portier doc[1].req: [%s] \n", doc1);
                  USE_SERIAL.printf("[IOc] gpio portier doc[1].todo: [%s] \n", doc2);
                  USE_SERIAL.printf("[IOc] gpio portier doc[1].idPorteChezPortier: [%i] \n", idPorte);
                  USE_SERIAL.printf("[IOc] gpio portier doc[1].gpio: [%i] \n", idGpio);

                   const char* jsonHostname = doc[1]["todo"];
                    if (jsonHostname)
                    {
                      Serial.print("gpio - jsonName todo existe: ");
                      Serial.println(jsonHostname);
                    }
                    else{
                      Serial.print("gpio - req not found ");
                    }
                    Serial.print("gpio - hostname = ");
                    Serial.println(hostname);
                    const char* jsonreq = doc[1]["req"];
                    //const char* jsonreq = jsonMess["req"];
                    if (jsonreq)
                    {   
                       Serial.print("gpio - req Trouve: ");
                       Serial.println(jsonreq);
                    }
                    USE_SERIAL.printf("[IOc] gpioTest: test le idGpio: %i ", idGpio);
                    USE_SERIAL.printf(", de gpio : %i \n", idGpio);
                    
                    if (triggeredDelay > delayOn)
                      triggerGPIO(idGpio, triggeredDelay);
                    else
                      triggerGPIO(idGpio);
                    delay(250);

                    //Answering
 // creat JSON message for Socket.IO (event)
                DynamicJsonDocument docAns(1024);
                JsonArray array = docAns.to<JsonArray>();
        
                // add evnet name
                // Hint: socket.on('event_name', ....
                array.add("message");
        
                // add payload (parameters) for the event
                JsonObject param1 = array.createNestedObject();
                //param1["req"] = "login";
                param1["id"] =  doc[1]["id"];
                param1["req"] = doc[1]["req"];
                param1["todo"] = "openRes";
                param1["pwd"] = pwd; //"J35u1sUnP0rT1e4-AAAA";
                param1["idSocketCaller"] = doc[1]["idSocketCaller"];
                param1["idCaller"] = doc[1]["idCaller"];
                param1["idPorte"] = doc[1]["idPorte"];
                //pour tricher en attendant
                myId = doc[1]["idPortier"].as<String>();
                if(myId == NULL || myId == "null")
                  myId=doc[1]["idCaller"].as<String>();
                  
                param1["idPortier"] = myId;
                param1["gpioUsed"] = gpioTrad[idGpio];
                param1["ents"] = doc[1]["ents"];
                param1["state"] = "4";
                //param1["date"] = (uint32_t) now;
                param1["date"] = DateTime.now();
        
                // JSON to String (serializion)
                String output;
                serializeJson(docAns, output);
        
                // Send event
                socketIO.sendEVENT(output);
        
                // Print JSON for debugging
                USE_SERIAL.println(output);

                    //-- answering
                }
                else if(eventName.compareTo("configuration") == 0)
                { 
                  Serial.println("configuration:");
                  DynamicJsonDocument docAns(1024);
                  JsonArray array = docAns.to<JsonArray>();
                    array.add("ack");
                    
                        // add payload (parameters) for the event
                        //JsonObject param1 = array.createNestedObject();
                        JsonObject param1 = array.createNestedObject();
                        param1["id"] =  doc[1]["id"];
                        param1["req"] = doc[1]["req"];
                        param1["todo"] = "answerConfiguration";
                        param1["what"] = "configuration";
                        param1["login"] = login ;
                        param1["pwd"] =pwd ;
                        param1["myId"] =myId ;
                        param1["amIRegistered"] =amIRegistered ;
                        param1["makeAPOn"] =makeAPOn ;
                        param1["what"] =maintenanceOn;
                        param1["maintenanceOn"] =loadPortier ;
                        param1["withSocketFallToWifi"] =withSocketFallToWifi ;
                        param1["inverted"] =inverted;
                        param1["gpioLed"] =gpioLed ;
                        param1["gpioLedStr"] = gpioLedStr;
                        param1["version"] = version;
                        param1["editingPassword"] = editingPassword;
                        param1["type"] = doc[1]["type"];
                        //param1["version"] = version;
                        param1["idSocketCaller"] = doc[1]["idSocketCaller"];
                        param1["idCaller"] = doc[1]["idCaller"];
                        //pour tricher en attendant
                        myId = doc[1]["idPortier"].as<String>();
                        param1["idPortier"] = myId;
                        param1["ents"] = doc[1]["ents"];
                        param1["state"] = "1";
                        param1["text"] = "are you happy?";
                        param1["date"] = DateTime.now();

              preferences.begin(prefNameSpace.c_str(), false);
          
//                        param1["pref_login"] = preferences.getString("login", "prefNotFound"); 
//                        param1["pref_pwd"] = preferences.getString("password", "prefNotFound") ;
//                        param1["pref_myId"] = preferences.getString("MY_ID", "prefNotFound") ;
//                        param1["pref_amIRegistered"] = preferences.getString("amIRegistered", "prefNotFound") ;
//                        param1["pref_makeAPOn"] = preferences.getBool("makeAPOn","prefNotFound");//.as<String>() ;
//                        param1["pref_maintenanceOn"] = preferences.getBool("maintenanceOn","prefNotFound");//.as<String>() ;
                        param1["pref_inverted"] = preferences.getString("inverted", "prefNotFound");     
                        String *tmpGpio = new String((preferences.getInt("gpioLed",9999)));
                        //param1["pref_gpioLed"] = tmpGpio->c_str(); //preferences.getString("gpioLed","prefNotFound"); //new String((preferences.getInt("gpioLed",9999)));//.as<String>() ; //new String(preferences.getInt("gpioLed", 999999)) ;
//                        param1["pref_editingPassword"] = preferences.getString("editingPassword", "prefNotFound");
  
              preferences.end();
                        // JSON to String (serializion)
                        String output;
                        serializeJson(docAns, output);
                
                        // Send event
                        socketIO.sendEVENT(output);
                
                        // Print JSON for debugging
                        Serial.println("socket configuration (get): sending:");
                        USE_SERIAL.println(output);
                }
                else if(eventName.compareTo("ack") == 0)
                {
                  USE_SERIAL.printf("[IOc]****** ACK ******\n");

                  String what = doc[1]["what"];
                  int state = doc[1]["state"].as<int>();
                  String mess = doc[1]["mess"];
                  
                  USE_SERIAL.printf("[IOc]ACK portier doc: %s \n", doc);
                  USE_SERIAL.printf("[IOc]ACK portier doc[1]-what: %s \n", what);
                  USE_SERIAL.printf("[IOc]ACK portier doc[2]-state: %i \n", state);
                  USE_SERIAL.printf("[IOc]ACK portier doc[3]-mess: %s \n", mess);
                  if( what.compareTo("login") == 0)
                  {
                    if(state == 1)
                    {
                      isRegistred = true;
                      USE_SERIAL.printf("[IOc]ACK Setting isRegistred to true\n");

                      USE_SERIAL.printf("[IOc]ACK Setting letState to connected: %i \n", SOCKET_CONNECTED);
                      ledState(SOCKET_CONNECTED);
                    }
                    else
                    {
                      USE_SERIAL.printf("[IOc]ACK login state is not 1 -> have to deal with it: %i \n", state);                      
                    }
                  }else
                  {
                      USE_SERIAL.printf("[IOc]ACK what is not 'login' -> have to deal with it: \n");
                  }
                }
                else
                {
                    USE_SERIAL.printf("[IOc] event != message  et portier\n");
                }
    
            break;
                }
        case sIOtype_ACK:
            USE_SERIAL.printf("[IOc] get ack: %u\n", length);
//            DynamicJsonDocument ack(1024);
//            DeserializationError errorAck = deserializeJson(ack, payload, length);
//            if(errorAck) {
//                USE_SERIAL.print(F("deserializeJson() failed: "));
//                USE_SERIAL.println(errorAck.c_str());
//                return;
//            }
//
//            String eventNameAck = ack[0];
//            String ack1Test = ack[1];
//            USE_SERIAL.printf("[IOc] event name: %s\n", eventNameAck.c_str());
//            //USE_SERIAL.printf("[IOc] doc[1] en c_str(): %s\n", doc1Test["req"]);
//              String ack1 = ack[1][0];
//              String ack2 = ack[1][1];
//              String ack3 = ack[1][2];
//              USE_SERIAL.printf("[IOc] ACK - doc: %s \n", ack);
//              USE_SERIAL.printf("[IOc] ACK - doc[1] %s \n", ack1);
//              USE_SERIAL.printf("[IOc] ACK - doc[2] %s \n", ack2);
//              USE_SERIAL.printf("[IOc] ACK - doc[3] %s \n", ack3);
//
//                if( eventNameAck.compareTo("message") == 0)
//                {
//                }
            break;
        case sIOtype_ERROR:
            USE_SERIAL.printf("[IOc] get error: %u\n", length);
            break;
        case sIOtype_BINARY_EVENT:
            USE_SERIAL.printf("[IOc] get binary: %u\n", length);
            //hexdump(payload, length);
//            HTTPUpload& upload = server.upload();
//                    if (upload.status == UPLOAD_FILE_START) {
//                      Update.begin(UPDATE_SIZE_UNKNOWN);
//                    } else if (upload.status == UPLOAD_FILE_WRITE) {
//                      Update.write(upload.buf, upload.currentSize);
//                    } else if (upload.status == UPLOAD_FILE_END) {
//                      Update.end(true);
//                    }
//                    if(!Update.end()){
//                        Serial.println("Error Occurred: " + String(Update.getError()));
//                        f.close();
//                        return false;
//                    }
            break;
        case sIOtype_BINARY_ACK:
            USE_SERIAL.printf("[IOc] get binary ack: %u\n", length);
            break;
    }
}



void checkNoWifiActionDone()
{
//const long intervalNoWifiActionCheck = 5000;
//unsigned long previousNoWifiActionCheck = 0;
//int iteNoWifiActionCheck = 0;
//unsigned int maxIteNoWifiActionBeforeReboot = 10;
//  ---
  unsigned long currentMillis = millis();

  if(onForXminBool || maintenanceOn)
  {
     if((currentMillis - previousNoWifiActionCheck)%2000 == 0)
    {
      Serial.print("[checkWifiNoActionDone] onForXminBool = [");Serial.print(onForXminBool);Serial.print("]: ");Serial.print(currentMillis);Serial.print("-");Serial.print(previousNoWifiActionCheck);
      Serial.print(" = ");Serial.print((currentMillis - previousNoWifiActionCheck));Serial.print(" / ");Serial.println(XminDelay);
    }
    if(currentMillis - previousNoWifiActionCheck >= XminDelay) {
       Serial.println("[checkWifiNoActionDone] XminDelay reached: should reboot");
        iteNoWifiActionCheck=0;
        makeAPOn=false;
        onForXminBool = false;
        maintenanceOn = false;
        preferences.begin(prefNameSpace.c_str(), false);
          preferences.putBool("makeAPOn", makeAPOn);
          preferences.putBool("maintenanceOn", maintenanceOn);
        preferences.end();
        ESP.restart();
      }
  }
  else if (currentMillis - previousNoWifiActionCheck >= intervalNoWifiActionCheck) {
      Serial.println("");
      Serial.print("[checkWifiNoActionDone] onForXminBool = [");Serial.print(onForXminBool);
      Serial.print("], currentMillis: ");Serial.println(currentMillis);

      previousNoWifiActionCheck = currentMillis;
      iteNoWifiActionCheck++;
      Serial.print("[checkWifiNoActionDone] making check ["); Serial.print(iteNoWifiActionCheck); Serial.print("/");Serial.println(maxIteNoWifiActionBeforeReboot);
      
      if (iteNoWifiActionCheck >= maxIteNoWifiActionBeforeReboot)
      {
        Serial.print("[checkWifiNoActionDone] should reboot");
        iteNoWifiActionCheck=0;
        makeAPOn=false;
        maintenanceOn = false;
        preferences.begin(prefNameSpace.c_str(), false);
          preferences.putBool("makeAPOn", makeAPOn);
          preferences.putBool("maintenanceOn", maintenanceOn);

        preferences.end();
        ESP.restart();
      }
       
    }
    else
    {
      //Serial.print(".");
    }
    //Serial.println("[checkWifiNoActionDone] end [ /5]");
}


void ActiverWebSocket()
{//si la connexion tombe soit avec internet, soit avec le serveur, est-ce que l'on ouvre l'AP?
  //faire que ca fonctionne.
  
    //USE_SERIAL.printf("[activerWebSocket] Activation de websocket pour %s, %s \n", hostname, hostnamePort);
    USE_SERIAL.println("[activerWebSocket] Activation de websocket pour on ne sait pas hostnae ni port");
    USE_SERIAL.println("[activerWebSocket] Activation de websocket pour");
    USE_SERIAL.println(socketHostname);
    USE_SERIAL.println(hostnamePort);

    //socketIO.setReconnectInterval(5000); // ca marche cela?
      
    // server address, port and URL
    //socketIO.begin("10.11.100.100", 8880, "/socket.io/?EIO=4");
    //socketIO.begin(socketHostname, hostnamePort, "/socket.io/?EIO=4");
    USE_SERIAL.println("[activerWebSocket]  apres le begin");
    // event handler
    socketIO.onEvent(socketIOEvent);
    USE_SERIAL.println("[activerWebSocket]  apres l'event");

    USE_SERIAL.print("[activerWebSocket]  begin avec: ");
    USE_SERIAL.print(socketHostname);
    USE_SERIAL.print(":");
    USE_SERIAL.print(hostnamePort);
    USE_SERIAL.println("/socket.io/?EIO=4");
    
    USE_SERIAL.print("[activerWebSocket]  begin ");
    if(isHTTPS)
    {
      USE_SERIAL.println("[activerWebSocket] socket begin en SSL ");
      socketIO.beginSSL(socketHostname, hostnamePort, "/socket.io/?EIO=4");
    }
    else
    {
      USE_SERIAL.println("[activerWebSocket] socket begin sans ssl ");
      socketIO.begin(socketHostname, hostnamePort, "/socket.io/?EIO=4");
    }
    USE_SERIAL.println("[activerWebSocket]  begin fait");

    //USE_SERIAL.print("[activerWebSocket]  event fournit");
    //USE_SERIAL.println(socketIOEvent.toString());



}




void setupByConfig()
{
  //use of preferences
    Serial.println("[setupByConfig] use of preferences...");

  preferences.begin(prefNameSpace.c_str(), false);
  if (preferences.getString("name", "").length() > 0 )
  {
    Serial.print("preferences.getstring name > 0 : changement de  nom.  longeur: ");
    Serial.println(preferences.getString("name", "").length());
    Serial.println(preferences.getString("name", ""));

    APname = "Ianitor - "+ preferences.getString("name", ""); //le concat ne fait pas du doute ce a quoi je m'attendais...
    Serial.print("new APname: ");Serial.println(APname);

  }
  ssid = preferences.getString("ssid", "");
  wifipass = preferences.getString("wifipass", "");
  wifimulti = preferences.getBool("wifimulti", false);
  //ip;
  //gateway = preferences.getString("gateway", "");
  hostname = preferences.getString("hostname", "");
  socketHostname = preferences.getString("socketHostname", "");
  //hostnamePort = preferences.getString("port", "").toInt();
  Serial.print("[setupByConfig] hostname = ");
  Serial.println(hostname);

  if(socketHostname.length() == 0 && hostname.length() > 0)
  {
    Serial.println("[setupByConfig] dans le if ");
  //Serial.println(hostname);
    splitHostnamePort(hostname, ":");
      preferences.putString("socketHostname", socketHostname);
      preferences.putInt("port", hostnamePort);   // vient avec le hostname.
  }
  //socketHostname = preferences.getString("socketHostname", ""); // se fait par la fonction du split
  //hostnamePort = preferences.getString("port", "").toInt();
  
  login = preferences.getString("login", "");
  pwd = preferences.getString("password", "");
  myId = preferences.getString("MY_ID", "fakeId");
  amIRegistered = preferences.getString("amIRegistered", "");
  makeAPOn = preferences.getBool("makeAPOn",makeAPOn);
  maintenanceOn = preferences.getBool("maintenanceOn",maintenanceOn);
  loadPortier = preferences.getBool("loadPortier", loadPortier);
  withSocketFallToWifi = preferences.getBool("withSocketFallToWifi", withSocketFallToWifi);
  inverted = preferences.getBool("inverted", inverted);
  gpioLed = preferences.getInt("gpioLed", gpioLed);
  gpioLedStr = preferences.getString("gpioLedStr", (new String(gpioLed))->c_str() );
  if(gpioLed == 0)
  {
    if(gpioLedStr != "0" && gpioLedStr != "notDefined")
    {
      gpioLed = gpioLedStr.toInt();
    }
  }
  editingPassword = preferences.getString("editingPassword", "");
  useStaticIp = preferences.getBool("useStaticIp", false);
  ip = preferences.getString("ip", "");
  gateway = preferences.getString("gateway", "");
  dns = preferences.getString("dns", "8.8.4.4");
  mask = preferences.getString("mask", "");

  useStaticIp_eth = preferences.getBool("useStaticIp_eth", false);
  ip_eth = preferences.getString("ip_eth", "");
  gateway_eth = preferences.getString("gateway_eth", "");
  dns_eth = preferences.getString("dns_eth", "8.8.4.4");
  mask_eth = preferences.getString("mask_eth", "");

  myPhy_addr = preferences.getInt("myPhy_addr", -1);
  
  isWt32 = preferences.getBool("isWt32", false);
  if(isWt32)
    useEth = preferences.getBool("useEth", true);
  else
    useEth = preferences.getBool("useEth", false);

  firmwareUpdated = preferences.getBool("firmwareUpdated", firmwareUpdated);
  if( firmwareUpdated )
  {
     updateFirmware_idComment = preferences.getInt("upFirm_idCom", 0);
     updateFirmware_idSocketCaller = preferences.getString("upFirm_idSocket", "unknown_IdSocketCaller");
     updateFirmware_idCaller = preferences.getString("upFirm_idCaller", "unknown_IdCaller");
     updateFirmware_type = preferences.getString("upFirm_type", "unknown_type");   
     updateFirmware_ents = preferences.getString("upFirm_ents", "unknown_ents");
     updateFirmware_oldVers = preferences.getString("upFirm_oldVers", "");
  }



  //if (ssid == "" || wifipass == "" || wifipass == "" || login == "" || pwd == "" || hostname == "" ){  je laisse les deux pour les tests de reconnection
  if (ssid == "" || wifipass == ""){
    if(useEth)
    {
      Serial.println("No ssid or password but useEth,");
      //makeAPOn = true;
   
    }
    else
    {
      Serial.println("No values anteriory saved for ssid or password");
      makeAPOn = true;
      onForXminBool = true;
    }
  }

  
//  if (LittleFS.begin()) { //utile pour les fichiers html
//      Serial.println("mounted file system");
//  }
//  else{
//    Serial.println("LittleFS not mounted: Pas d'affichage des pages");
//  }
  preferences.end();
  
}


void configStaticIp_eth(bool _useStaticIp)
{
  if (_useStaticIp)
  {
    bool isOk = true;

    if (myIp.fromString(ip_eth)) { // try to parse into the IPAddress
        Serial.print("configStaticIp_eth: myIp:"); // print the parsed IPAddress 
        Serial.println(myIp);
        
    } else {
        isOk = false;
        Serial.println("UnParsable IP");
    }
    if (myGW.fromString(gateway_eth)) { // try to parse into the IPAddress
        Serial.print("configStaticIp_eth: myGW:"); // print the parsed IPAddress 
        Serial.println(myGW);
    } else {
      isOk = false;
        Serial.println("UnParsable myGW");
    }
    if (mySN.fromString(mask_eth)) { // try to parse into the IPAddress
        Serial.print("configStaticIp_eth: mySN:"); // print the parsed IPAddress 
        Serial.println(mySN);
    } else {
      isOk = false;
        Serial.println("UnParsable mySN");
    }
    if (myDNS.fromString(dns_eth)) { // try to parse into the IPAddress
        Serial.print("configStaticIp_eth: myDNS:"); // print the parsed IPAddress 
        Serial.println(myDNS);
    } else {
      isOk = false;
        Serial.println("configStaticIp_eth: UnParsable myDNS");
    }
    if(isOk)
    {
      if(useEth)
      {
        Serial.println("configStaticIp_eth: useETH, setting eth.config");
        if(!ETH.config(myIp, myGW, mySN, myDNS))
        {
          Serial.println("configStaticIp_eth: ETH configured");        
        }
        else
          Serial.println("configStaticIp_eth: ETH FAILED to configure");

      }
      else{
        Serial.println("configStaticIp_eth: no static Ip_eth configured: useEth is false");
      }
    }
    else{
      Serial.println("configStaticIp_eth: not configuring as one address id not parsable");
    }
  }
  else
  {
     Serial.println("configStaticIp_eth: no Static IP_eth required");

  }
}

void configStaticIp(bool _useStaticIp)
{
  if (_useStaticIp)
  {
    bool isOk = true;

    if (myIp.fromString(ip)) { // try to parse into the IPAddress
        Serial.print("myIp:"); // print the parsed IPAddress 
        Serial.println(myIp);
        
    } else {
        isOk = false;
        Serial.println("UnParsable IP");
    }
    if (myGW.fromString(gateway)) { // try to parse into the IPAddress
        Serial.print("myGW:"); // print the parsed IPAddress 
        Serial.println(myGW);
    } else {
      isOk = false;
        Serial.println("UnParsable myGW");
    }
    if (mySN.fromString(mask)) { // try to parse into the IPAddress
        Serial.print("mySN:"); // print the parsed IPAddress 
        Serial.println(mySN);
    } else {
      isOk = false;
        Serial.println("UnParsable mySN");
    }
    if (myDNS.fromString(dns)) { // try to parse into the IPAddress
        Serial.print("myDNS:"); // print the parsed IPAddress 
        Serial.println(myDNS);
    } else {
      isOk = false;
        Serial.println("UnParsable myDNS");
    }
    if(isOk)
    {
      if(!WiFi.config(myIp, myGW, mySN, myDNS)) {
      Serial.println("configStaticIp: STA Failed to configure");
      }
    }
    else{
      Serial.println("configStaticIp: not configuring as one address id not parsable");
    }
  }
  else
  {
     Serial.println("configStaticIp: no Static IP required");

  }
}
// Initialize WiFi
bool initWiFi() {
    ledState(TRYING_WIFI);  
    unsigned long currentMillis = millis();
    previousMillis = currentMillis;
  
    for(uint8_t t = 4; t > 0; t--) {
        USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
        USE_SERIAL.flush();
        delay(1000);
    }
    if(useEth && isWt32)
    {
        Serial.print("\n [SETUP] wt32_eth01 Starting BasicHttpsClient on " + String(ARDUINO_BOARD));
        Serial.println(" with " + String(SHIELD_TYPE));
              
        Serial.printf("ESP32 Chip model = %s Rev %d\n", ESP.getChipModel(), ESP.getChipRevision());
        Serial.printf("This chip has %d cores\n", ESP.getChipCores());
        Serial.printf("This chip mac address: %d \n", ESP.getEfuseMac());

        Serial.print("WEBSERVER_WT32_ETH01_VERSION: ");
        Serial.println(WEBSERVER_WT32_ETH01_VERSION);
      
        Serial.print("ETH_PHY_ADDR: ");
        Serial.println(ETH_PHY_ADDR);
        Serial.print("ETH_PHY_POWER: ");
        Serial.println(ETH_PHY_POWER);
        Serial.print("eth going with myPhy_addr: ");
        Serial.println(myPhy_addr);
        //ETH_PHY_ADDR = 1;
        // To be called before ETH.begin()
        WT32_ETH01_onEvent();
//        ETH.begin(ETH_PHY_ADDR, ETH_PHY_POWER);

        ETH.begin(myPhy_addr, ETH_PHY_POWER);
        configStaticIp_eth(useStaticIp_eth); //aurait besoin d'etre apres le begin contrairement au wifi

        
        //WT32_ETH01_waitForConnect();
        for(int i = 0; i < 20 && !WT32_ETH01_isConnected(); i++) {
            currentMillis = millis();
  
          USE_SERIAL.printf("[SETUP] wt32_eth01 is connected: attempt[%d/20] still not Connected: ", i);
          USE_SERIAL.print(WT32_ETH01_isConnected());
          USE_SERIAL.println("");
           delay(500);
        }
        if (!WT32_ETH01_isConnected())
        {
              Serial.println("[SETUP] wt32_eth01 is not connected: Failed to connect or unplugged cable?");
              Serial.println("[SETUP] wt32_eth01 keep on with wifi");
              //return false;
        }else
        {
          String ipTemp = ETH.localIP().toString();
          USE_SERIAL.printf("[SETUP] wt32_eth01 Connected %s\n", ipTemp);
          return true;
        }
    }
    WiFi.mode(WIFI_STA);
//    if (useStaticIp)
//    {
//      if(!WiFi.config(myIp, myGW, mySN, myDNS)) {
//        Serial.println("STA Failed to configure");
//      }
//    }
    if(wifimulti)
    {
       configStaticIp(useStaticIp);
       WiFiMulti.addAP(ssid.c_str(), wifipass.c_str());
      
      //WiFi.disconnect();
      USE_SERIAL.printf("[SETUP] wifi multi: Connecting to wifi");
      //beep(300);
      //while(WiFiMulti.run() != WL_CONNECTED) {
      for(int i = 0; i < 20 && WiFiMulti.run() != WL_CONNECTED; i++) {
            currentMillis = millis();
  
          USE_SERIAL.printf("[SETUP] wifi multi: attempt[%d/20] still not Connected: ", i);
          USE_SERIAL.print(wl_status_to_string(WiFiMulti.run()));
          USE_SERIAL.println("");
  
          //USE_SERIAL.printf(".");
  //        if (currentMillis - previousMillis >= interval) {
  //          Serial.println("[SETUP] wifi: Failed to connect.");
  //          return false;
  //        }
          delay(500);
          //beep(300);
  
      }
      if (WiFiMulti.run() != WL_CONNECTED)
      {
            Serial.println("[SETUP] wifi multi: Failed to connect, trying with standard wifi for 5 times");
             USE_SERIAL.printf("[SETUP] wifi: Connecting to wifi");
             configStaticIp(useStaticIp);
             WiFi.begin(ssid.c_str(), wifipass.c_str());
             
            //beep(300);
            //while(WiFiMulti.run() != WL_CONNECTED) {
            for(int i = 0; i < 5 && WiFi.status() != WL_CONNECTED; i++) {
                  currentMillis = millis();
        
                USE_SERIAL.printf("[SETUP] wifi: attempt[%d/20] still not Connected: ", i);
                USE_SERIAL.print(get_wifi_status(WiFi.status()));
                USE_SERIAL.println("");
        
                //USE_SERIAL.printf(".");
        //        if (currentMillis - previousMillis >= interval) {
        //          Serial.println("[SETUP] wifi: Failed to connect.");
        //          return false;
        //        }
                delay(500);
                //beep(300);
        
            }
            if (WiFi.status() != WL_CONNECTED)
            {
                  Serial.println("[SETUP] wifi: Failed to connect.");
                  Serial.print("[SETUP] wifi: status: ");
                   Serial.println(get_wifi_status(WiFi.status()));
                  return false;
            }
            //return false;
      }
    }else
    {
       USE_SERIAL.printf("[SETUP] wifi: Connecting to wifi");
       configStaticIp(useStaticIp);
       WiFi.begin(ssid.c_str(), wifipass.c_str());
      //beep(300);
      //while(WiFiMulti.run() != WL_CONNECTED) {
      for(int i = 0; i < 20 && WiFi.status() != WL_CONNECTED; i++) {
            currentMillis = millis();
  
          USE_SERIAL.printf("[SETUP] wifi: attempt[%d/20] still not Connected: ", i);
          USE_SERIAL.print(get_wifi_status(WiFi.status()));
          USE_SERIAL.println("");
  
          //USE_SERIAL.printf(".");
  //        if (currentMillis - previousMillis >= interval) {
  //          Serial.println("[SETUP] wifi: Failed to connect.");
  //          return false;
  //        }
          delay(500);
          //beep(300);
  
      }
      if (WiFi.status() != WL_CONNECTED)
      {
            Serial.println("[SETUP] wifi: Failed to connect.");
            Serial.print("[SETUP] wifi: status: ");
             Serial.println(get_wifi_status(WiFi.status()));
            return false;
      }
    }
    String ipTemp = WiFi.localIP().toString();
    USE_SERIAL.printf("[SETUP] WiFi Connected %s\n", ipTemp.c_str());
    return true;

}


String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;
    
  // Your Domain name with URL path or IP address with path
  http.begin(client, serverName);
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
  if (httpResponseCode>0) {
    Serial.printf("httpGETRequest(%s)HTTP Response code: ",serverName);
    Serial.println(httpResponseCode);
    payload = http.getString();
    Serial.printf("httpGETRequest(%s)HTTP Response payload: ",serverName);
    Serial.println(payload);
  }
  else {
    Serial.print("httpGETRequest: Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}


void loadFromWeb()
{
      Serial.println("loadFromWeb: call ");

  //++++ getting ipExt
//  String ipExt = "1";
//  String ipInt = "1";
//      Serial.print("loadFromWeb: wifiMultiRun(): ");
//      Serial.println(WiFiMulti.run());
//      Serial.print(" : ");
//      Serial.println(wl_status_to_string(WiFiMulti.run()));
  if(useEth && isWt32)
  {
      Serial.print("loadFromWeb: eth status(): is connected? ");
      Serial.println(WT32_ETH01_isConnected());

  }else{
      Serial.print("loadFromWeb: wifi status(): ");
      Serial.print(get_wifi_status(WiFi.status()));
      Serial.print(" : ");
      Serial.println(WiFi.status());
  }
      
  if(WiFi.status() == WL_CONNECTED || WiFiMulti.run() == WL_CONNECTED || WT32_ETH01_isConnected() )
  {
    if(useEth && isWt32)
    {
      if (WT32_ETH01_isConnected())
        ipInt = ETH.localIP().toString();
      else
        ipInt = WiFi.localIP().toString();
    }
    else
      ipInt = WiFi.localIP().toString();
    
    Serial.println("loadfromWeb : essaie d'avoir l'adresse ipExt - pas besoind e le faire, devratie tetre fait par une autre fonction ");
//       HTTPClient http;
//       String serverPath = "https://www.whatismyip.com/fr/";
//      
//      // Your Domain name with URL path or IP address with path
//      http.begin(serverPath.c_str());
//      
//      // Send HTTP GET request
//      int httpResponseCode = http.GET();
//      
//      if (httpResponseCode>0) {
//        Serial.print("HTTP Response code: ");
//        Serial.println(httpResponseCode);
//        String payload = http.getString();
//        Serial.println(payload); 
//
//      }
//      else {
//        Serial.print("whatismyip.com Error code: ");
//        Serial.println(httpResponseCode);
//      }
//      // Free resources
//      http.end();
  }
  else
  {
    if(useEth && isWt32)
    {
        Serial.print("loadFromWeb: eth status(): is connected? ");
        Serial.print(WT32_ETH01_isConnected());
  
    }else{
      Serial.print("loadFromWeb: wifi status(): ");
      Serial.print(get_wifi_status(WiFi.status()));
      Serial.print(" : ");
      Serial.println(WiFi.status());
      
      Serial.print("loadFromWeb: wifiMultiRun(): ");
      Serial.println(WiFiMulti.run());
      Serial.print(" : ");
      Serial.println(wl_status_to_string(WiFiMulti.run()));
        
      Serial.println("loadfromWeb : pas de wifi, pas d'adresse ipExt ");
    }
  }
   Serial.print("ipInt = ");
    Serial.println(ipInt);
     Serial.print("ipExt = ");
     Serial.println(ipExt);
//---- getting ipExt

      Serial.println("-----");
      Serial.println("loadFromWeb: looking for esp32 config ");

          //const char* serverName = "http://192.168.43.187:8082/registersrv/api/register?login="+login+"&pwd="+pwd+"&ipInt="+WiFi.localIP()+"&ipExt=1&stringFormat=json";
          //sensorReadings = httpGETRequest(serverName);
          //String ipLocale = WiFi.localIP().toString();
          //String serverName = "http://192.168.43.187:8082/registersrv/api/register?login="+login+"&pwd="+pwd+"&ipInt="+(WiFi.localIP().toString())+"&ipExt="+ipExt+"&stringFormat=json";
          String serverName = hostname+"/registersrv/api/register?login="+login+"&pwd="+pwd+"&ipInt="+ipInt+"&ipExt="+ipExt+"&stringFormat=json";
          Serial.print("serverName = ");
          Serial.println(serverName);
          String sensorReadings;
          String sensorReadingsArr[13];
          //sensorReadings = httpGETRequest(serverName.c_str());
          HTTPClient http;
          http.begin(serverName.c_str());
      
            // Send HTTP GET request
            int httpResponseCode = http.GET();
            
            if (httpResponseCode>0) {
              Serial.print("HTTP Response code: ");
              Serial.println(httpResponseCode);
                sensorReadings = http.getString();
                  Serial.print("reponse du serveur: ");
                  Serial.println(sensorReadings);
  
              /***************************************************
              /******** +++++ DEVRAIT faire cela +++++ ***********
              // Allocate the JSON document
              // Use arduinojson.org/v6/assistant to compute the capacity.
              //  const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
              //  DynamicJsonDocument doc(capacity);
              /********  ---- DEVRAIT faire cela ---- ***********/
              
              DynamicJsonDocument docSensorReadings(1024);
              DeserializationError error = deserializeJson(docSensorReadings, sensorReadings);
              if(error) {
                  USE_SERIAL.print(F("deserializeJson() failed: "));
                  USE_SERIAL.println(error.c_str());
                  makeAPOn = true;
                  return;
              }
                  
                  
                  // myObject.keys() can be used to get an array of all the keys in the object
                 // String keys[] = docSensorReadings.keys();
  
                 JsonObject root=docSensorReadings.as<JsonObject>();
                 String value = "";
                 int iterator = 0;
                  for (JsonPair kv : root) {
                      //value = kv.value().as<char*>();
                      value = docSensorReadings[kv.key().c_str()].as<char*>();
                      Serial.printf("%d - cle: ", iterator);
                      Serial.println(kv.key().c_str());
                      Serial.print("value: ");
                      
                      Serial.println(value);
  //                    Serial.println(kv.value().as<char*>());
  //                    value = kv.value().as<char*>();
                      sensorReadingsArr[iterator] = value;
                      iterator++;
                  }
  
                
  //                for (int i = 0; i < keys.length(); i++) {
  //                  String value = docSensorReadings[keys[i]];
  //                  Serial.print(keys[i]);
  //                  Serial.print(" = ");
  //                  Serial.println(value);
  //                  sensorReadingsArr[i] = value;
  //                }
  
  
  //              sensorReadingsArr[0] = docSensorReadings["hostname"].as<char*>();
  //              sensorReadingsArr[1] = docSensorReadings["delete_login"].as<char*>();
  //              sensorReadingsArr[2] = docSensorReadings["delete_pass"].as<char*>();
  //              sensorReadingsArr[3] = docSensorReadings["login"].as<char*>();
  //              sensorReadingsArr[4] = docSensorReadings["password"].as<char*>();
  //              sensorReadingsArr[5] = docSensorReadings["notifyTechSRV"].as<char*>();
  //              sensorReadingsArr[6] = docSensorReadings["freespaceWarning"].as<char*>();
  //              sensorReadingsArr[7] = docSensorReadings["KEYPASSREAD"].as<char*>();
  //              sensorReadingsArr[8] = docSensorReadings["KEYPASSDEL"].as<char*>();
  //              sensorReadingsArr[9] = docSensorReadings["KEYPASSDONE"].as<char*>();
  //              sensorReadingsArr[10] = docSensorReadings["gpioController"].as<char*>();
  //              sensorReadingsArr[11] = docSensorReadings["MY_ID"].as<char*>();
  //              sensorReadingsArr[12] = docSensorReadings["name"].as<char*>();
  //              //sensorReadingsArr[13] = docSensorReadings["hostname"].as<char*>();
  
  
  
                 preferences.begin( prefNameSpace.c_str(), false);
                  //preferences.putString("ssid", ssid); 
                  //preferences.putString("wifipass", wifipass);
                  preferences.putString("name", docSensorReadings["name"].as<char*>());
                  //preferences.putString("hostname", docSensorReadings["hostname"].as<char*>());  //pas besoin, il est donne par l'utilisateur avec l'AP
  
  
                  splitHostnamePort(docSensorReadings["hostname"].as<char*>(), ":");
                    preferences.putString("socketHostname", socketHostname);
                    preferences.putInt("port", hostnamePort);   // vient avec le hostname.
                  //preferences.putString("login", docSensorReadings["login"].as<char*>());
                  //preferences.putString("password", docSensorReadings["password"].as<char*>());
                  preferences.putString("MY_ID", docSensorReadings["myId"].as<char*>());
                  preferences.putString("amIRegistered", docSensorReadings["amIRegistered"].as<char*>());
                  preferences.putInt("gpioLed", docSensorReadings["gpioLed"].as<int>());   
                  preferences.putString("gpioLedStr", docSensorReadings["gpioLed"].as<char*>());
                  preferences.putBool("inverted", inverted);

                Serial.print("loadFromWeb : changement GPIOLED et inverted:");
                Serial.print("as char:");Serial.print(docSensorReadings["gpioLed"].as<char*>());
                 Serial.print(" - as int:");Serial.print(docSensorReadings["gpioLed"].as<int>());
                Serial.print(", ");
                Serial.println(inverted);
              Serial.print(serverName);
  
                  
                 preferences.end();
                 
  //                Serial.print("1 = ");
  //                Serial.println(sensorReadingsArr[0]);
  //                Serial.print("2 = ");
  //                Serial.println(sensorReadingsArr[1]);
  //                Serial.print("3 = ");
  //                Serial.println(sensorReadingsArr[2]);
            }
            else
            {
              makeAPOn = true;
              preferences.begin(prefNameSpace.c_str(), false);
                preferences.putBool("makeAPOn",makeAPOn);
              preferences.end();
              Serial.print("Error code pour : ");
              Serial.print(serverName);
              Serial.println(": ");
              Serial.println(httpResponseCode);
              Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpResponseCode).c_str());
            }
            http.end();
}

static bool eth_connected = false;
void WiFiEvent(WiFiEvent_t event)
{
  switch (event) {
    case ARDUINO_EVENT_ETH_START:
      Serial.println("WiFiEvent - ETH Started");
      Serial.println("WiFiEvent - setting hostname: esp32-ethernet");
      //set eth hostname here
      ETH.setHostname("esp32-ethernet");
      break;
    case ARDUINO_EVENT_ETH_CONNECTED:
      Serial.println("WiFiEvent - ETH Connected");
      break;
    case ARDUINO_EVENT_ETH_GOT_IP:
      Serial.print("WiFiEvent - ETH MAC: ");
      Serial.print(ETH.macAddress());
      Serial.print(", IPv4: ");
      Serial.print(ETH.localIP());
      if (ETH.fullDuplex()) {
        Serial.print(", FULL_DUPLEX");
      }
      Serial.print(", ");
      Serial.print(ETH.linkSpeed());
      Serial.println("Mbps");
      eth_connected = true;
      break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
      Serial.println("WiFiEvent - ETH Disconnected");
      eth_connected = false;
      break;
    case ARDUINO_EVENT_ETH_STOP:
      Serial.println("WiFiEvent - ETH Stopped");
      eth_connected = false;
      break;
    default:
      break;
  }
}

void setup() {
    //USE_SERIAL.begin(921600);
    USE_SERIAL.begin(115200);

    //Serial.setDebugOutput(true);
    USE_SERIAL.setDebugOutput(true);

    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println("++++++++++++++++++++++++++++++++++++++++");
    USE_SERIAL.println("++++++++++  nv demarrage +++++++++++++++");
    USE_SERIAL.println();

    Serial.printf("ESP32 Chip model = %s Rev %d\n", ESP.getChipModel(), ESP.getChipRevision());
  Serial.printf("This chip has %d cores\n", ESP.getChipCores());
  Serial.print("\n Board: " + String(ARDUINO_BOARD));
  Serial.println(" shield: " + String(SHIELD_TYPE));
    Serial.print("version: "); 
      Serial.println(version);
    //initLittleFS();
    Serial.println("avant setupbyconfig:");

    Serial.print("ssid: "); 
      Serial.println(ssid);
    Serial.print("wifipass: ");
    Serial.println(wifipass);
    Serial.print("wifimulti: ");
    Serial.println(wifimulti);
    Serial.print("name: ");
    Serial.println(APname);
    Serial.print("hostname: ");
    Serial.println(hostname);
    Serial.print("login: ");
    Serial.println(login);
    Serial.print("pwd: ");
    Serial.println(pwd);
    Serial.print("makeAPOn: ");
    Serial.println(makeAPOn);
    Serial.print("maintenanceOn: ");
    Serial.println(maintenanceOn);
    Serial.print("myId: ");
    Serial.println(myId);
    Serial.print("editingPassword: ");
    Serial.println(editingPassword);
    Serial.print("gpioLed: ");
    Serial.println(gpioLed);
    Serial.print("gpioLedStr: ");
    Serial.println(gpioLedStr);
    Serial.print("forceWt32: ");
    Serial.println(forceWt32);
    Serial.print("isWt32: ");
    Serial.println(isWt32);
    Serial.print("useEth: ");
    Serial.println(useEth);
    Serial.print("useStaticIp: ");
    Serial.println(useStaticIp);
    Serial.print("myPhy_addr: ");
    Serial.println(myPhy_addr);
    Serial.print("phy_addr_default: ");
    Serial.println(phy_addr_default);

  setupByConfig();
    Serial.println("-*-*-*-*-* apres setupbyconfig:*-*-*-*-*-");
//    Serial.print("hostname: ");
//    Serial.print(hostname);
//    Serial.print(", login: ");
//    Serial.print(login);
//    Serial.print(" pwd: ");
//    Serial.println(pwd);

  // Set GPIO 2 as an OUTPUT
    pinInit();
    ledInit();
    Serial.println("pinInit: setting led OFF for disconnected");
    ledState(SOCKET_DISCONNECTED);

    if(forceWt32)
    {
      isWt32 = true;
      useEth = true;
    }else
    {
//      Serial.print("checking Shield, is it SHIELD_TYPE.indexOf('ETH_PHY_LAN8720') = ");
////      String shieldType = new String(*SHIELD_TYPE);
////      Serial.println(shieldType.indexOf("ETH_PHY_LAN8720"));
////      if( shieldType.indexOf("ETH_PHY_LAN8720") == 0 )
//      
//      Serial.println((String(SHIELD_TYPE)).indexOf("ETH_PHY_LAN8720"));
//      if( (String(SHIELD_TYPE)).indexOf("ETH_PHY_LAN8720") == 0 )
//        isWt32 = true;
//        if(phy_addr_default  == -1)
//        {
//          phy_addr_default = ETH_PHY_ADDR;
//          preferences.begin(prefNameSpace.c_str(), false);
//            preferences.putInt("phy_addr_default", phy_addr_default);
//          preferences.end();
//        }
//        if(myPhy_addr == -1)
//            myPhy_addr = phy_addr_default;      
    }
    
    if(isWt32)
    {
      Serial.print("checking Shield, is it SHIELD_TYPE.indexOf('ETH_PHY_LAN8720') = ");
//      String shieldType = new String(*SHIELD_TYPE);
//      Serial.println(shieldType.indexOf("ETH_PHY_LAN8720"));
//      if( shieldType.indexOf("ETH_PHY_LAN8720") == 0 )
      
      Serial.println((String(SHIELD_TYPE)).indexOf("ETH_PHY_LAN8720"));
      if( (String(SHIELD_TYPE)).indexOf("ETH_PHY_LAN8720") == 0 )
        isWt32 = true;
        if(phy_addr_default  == -1)
        {
          phy_addr_default = ETH_PHY_ADDR;
          preferences.begin(prefNameSpace.c_str(), false);
            preferences.putInt("phy_addr_default", phy_addr_default);
          preferences.end();
        }
        if(myPhy_addr == -1)
            myPhy_addr = phy_addr_default;
        WiFi.onEvent(WiFiEvent);

    }
    // Load values saved in LittleFS
    //ssid = readFile(LittleFS, ssidPath);
    //wifipass = readFile(LittleFS, wifipassPath);
    Serial.print("ssid: "); 
      Serial.println(ssid);
    Serial.print("wifipass: ");
    Serial.println(wifipass);
    Serial.print("wifimulti: ");
    Serial.println(wifimulti);
    Serial.print("name: ");
    Serial.println(APname);
    Serial.print("hostname: ");
    Serial.println(hostname);
    Serial.print("login: ");
    Serial.println(login);
    Serial.print("pwd: ");
    Serial.println(pwd);
    Serial.print("makeAPOn: ");
    Serial.println(makeAPOn);
    Serial.print("maintenanceOn: ");
    Serial.println(maintenanceOn);    
    Serial.print("loadPortier: ");
    Serial.println(loadPortier);
    Serial.print("withSocketFallToWifi: ");
    Serial.println(withSocketFallToWifi);
    Serial.print("myId: ");
    Serial.println(myId);
    Serial.print("editingPassword: ");
    Serial.println(editingPassword);
    Serial.print("gpioLed: ");
    Serial.println(gpioLed);
    Serial.print("gpioLedStr: ");
    Serial.println(gpioLedStr);
    Serial.print("forceWt32: ");
    Serial.println(forceWt32);
    Serial.print("isWt32: ");
    Serial.println(isWt32);
    Serial.print("useEth: ");
    Serial.println(useEth);
    Serial.print("useStaticIp: ");
    Serial.println(useStaticIp);
    Serial.print("myPhy_addr: ");
    Serial.println(myPhy_addr);
    Serial.print("phy_addr_default: ");
    Serial.println(phy_addr_default);
    Serial.print("firmwareUpdated: ");
    Serial.println(firmwareUpdated);
    
    
    
    if(!makeAPOn)
    {
        Serial.print("!makeAPOn est vraidonc makeApon est FAUX  ");
        Serial.println(makeAPOn);
         
    }
    if(makeAPOn)
    {
        Serial.print("makeAPOn est vrai donc makeApon est VRAI  ");
        Serial.println(makeAPOn);
         
    }
    if(!makeAPOn && initWiFi()){

      //Serial.println("getting ext ipAddress");
      //GetExternalIP();
      
      Serial.println("wifi connecte, on va chercher l'heure NTP");
      setupDateTime();

      //Serial.println("test updateFromWeb");
      //updateFromWeb("Esp321547895","1.4.5.7.8.5.3");
  
      Serial.print("[setupByConfig] loadPortier = ");
      Serial.println(loadPortier);
  
      if(loadPortier)
      {
        loadFromWeb();
      }

      
      Serial.println("wifi connecte, on va activer le websocket");

      ActiverWebSocket();
      //delay(1000);
      //loadFromWeb();
    }
    else
    {
      ledState(ACCESS_POINT_ON);
    Serial.println("recherche des wifis existants");
      getWifis();
    Serial.println("Activer l'AP, voir comment c'est possible avec wifimulti");
    WiFi.disconnect();

    // Connect to Wi-Fi network with SSID and password
    Serial.println("Setting AP (Access Point)");

    Serial.print("wifi mode [Avant]: ");Serial.println(WiFi.getMode());

    WiFi.mode(WIFI_AP);
    Serial.print("wifi mode [Apres]: ");Serial.println(WiFi.getMode());
    // NULL sets an open Access Point

    //Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");
    WiFi.softAP(APname.c_str(), NULL);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP); 

//    if (LittleFS.begin()) {
//        Serial.println("mounted file system");
//    }
//    else{
//      Serial.println("LittleFS not mounted: Pas d'affichage des pages");
//      }

    // Web Server Root URL
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      iteNoWifiActionCheck=0;
      
      int params = request->params();
      if(request->hasParam(PARAM_GET_1))
      {//implique que c'est un GET : https://github.com/me-no-dev/ESPAsyncWebServer -> "//Check if GET parameter exists"
        Serial.print("serveur:/ : get: a le parametre [");Serial.print(PARAM_GET_1);Serial.println("]");
        previousNoWifiActionCheck = millis();
        onForXminBool=true;
        Serial.print("serveur:/ : get: onForXminBool = [");Serial.print(onForXminBool);Serial.println("]");

      }
      else{
        Serial.print("serveur:/ : get: n'a pas le parametre [");Serial.print(PARAM_GET_1);Serial.println("]");
        onForXminBool=false;
        Serial.print("serveur:/ : get: setting to false: onForXminBool = [");Serial.print(onForXminBool);Serial.println("]");
        
      
        for(int i=0;i<params;i++){
        AsyncWebParameter* p = request->getParam(i);   
        if(!p->isPost() && p->isFile()){
             if (p->name() == PARAM_GET_1) {
              previousNoWifiActionCheck = millis();
              onForXminBool=true;
             }
             else{
              Serial.print("serveur:/ : get: parameter[");
              Serial.print(i);
              Serial.print("] =[");
              Serial.print(p->name());
              Serial.println("]");
             }

          }
      }
      }
      Serial.println("should get wifimanager.html");

//const char index_html[] PROGMEM = "..."; // large char array, tested with 14k
//request->send_P(200, "text/html", index_html);

//const char index_html[] PROGMEM = wifimanagerHTML.c_str(); // large char array, tested with 14k
request->send_P(200, "text/html", wifimanagerHTML.c_str());
      
      //request->send(LittleFS, "/wifimanager.html", "text/html");
      Serial.println("should get wifimanager.html: done");

    });
    
    //server.serveStatic("/", LittleFS, "/");

   server.on("/clearPreferences", HTTP_GET, [](AsyncWebServerRequest *request){
      Serial.println("Clear preferences: should clear preferences");
      preferences.begin(prefNameSpace.c_str(), false);
        size_t whatsLeft = preferences.freeEntries();    // this method works regardless of the mode in which the namespace is opened.
        Serial.printf("There are: %u entries available in the namespace table.\n", whatsLeft);

        preferences.clear();
        whatsLeft = preferences.freeEntries();    // this method works regardless of the mode in which the namespace is opened.
        Serial.printf("Clear preferences: done -> There are: %u entries available in the namespace table.\n", whatsLeft);
        editingPassword = "";
      preferences.end();

      String varToJSon = "let myResult = {\"result\":\"preferences cleared\"}";
      request->send(200, "application/javascript", varToJSon.c_str());
    });
    
    server.on("/getvariables.js", HTTP_GET, [](AsyncWebServerRequest *request){
      Serial.println("should send variables");
      String editingPwdSet = "false";
//      if(!editingPassword.isEmpty())
//        editingPwdSet="true";
      String varToJSon = "let myVars = {\"editingPwdSet\":\""+(editingPassword.isEmpty()?String(0):String(1) )+"\",\"version\":\""+version+"\",\"delayBeforeApDown\":\""+(onForXminBool?String(XminDelay):String(intervalNoWifiActionCheck*maxIteNoWifiActionBeforeReboot))+"\",\"ssid\":\""+ssid+"\",\"wifipass\":\"kept for me\",\"hostname\":\""+hostname+"\",\"login\":\""+login+"\",\"pwd\":\""+pwd+"\",\"useStaticIp\":\""+useStaticIp+"\",\"ip\":\""+ip+"\",\"gateway\":\""+gateway+"\",\"dns\":\""+dns+"\",\"mask\":\""+mask+"\",\"useEth\":\""+useEth+"\",\"myPhy_addr\":\""+String(myPhy_addr)+"\",\"phy_addr_default\":\""+String(phy_addr_default)+"\",\"ip_eth\":\""+ip_eth+"\",\"gateway_eth\":\""+gateway_eth+"\",\"dns_eth\":\""+dns_eth+"\",\"mask_eth\":\""+mask_eth+"\",\"useStaticIp_eth\":\""+useStaticIp_eth+"\",\"isWt32\":\""+isWt32+"\",\"macAddress\":\""+ETH.macAddress()+"\"}";

//      AsyncJsonResponse * response = new AsyncJsonResponse();
//      response->addHeader("Server","ESP Async Web Server");
//      JsonObject& root = response->getRoot();
//      root["delayBeforeApDown"] = String(XminDelay);
//      root["ssid"] = ssid;
//      root["wifipass"] = wifipass;
//      root["hostname"] = hostname;
//      root["pwd"] = pwd;
//      response->setLength();
//      request->send(response);
      request->send(200, "application/javascript", varToJSon.c_str());
      Serial.println("should send variables: done");

    });
    
   server.on("/getwifis.js", HTTP_GET, [](AsyncWebServerRequest *request){
    String wifiToJSonTSend = "let myWifis = "+wifiToJSon;
//    int n = WiFi.scanNetworks();
//    Serial.println("scan done");
//    
//    if (n == 0) {
//        Serial.println("no networks found");
//    } else {
//      Serial.print(n);
//      Serial.println(" networks found");
//      for (int i = 0; i < n; ++i) {
//        // Print SSID and RSSI for each network found
//        wifiToJSon += "{\"id\":"+String(i+1)+",\"ssid\":\""+WiFi.SSID(i)+"\",\"rssi\":\""+WiFi.RSSI(i)+"\",\"encryption\":\""+((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*")+"\"}";
//        if(i < (n-1))
//          wifiToJSon += ",";
//        Serial.print(i + 1);
//        Serial.print(": ");
//        Serial.print(WiFi.SSID(i));
//        Serial.print(" (");
//        Serial.print(WiFi.RSSI(i));
//        Serial.print(")");
//        Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
//      }
//    }
//     wifiToJSon += "]";
    Serial.println("wifi to json to send: ");
     Serial.println(wifiToJSonTSend);
     wifiToJSonTSend+=";console.log('getwifis.js : variable importee');";
    request->send(200, "application/javascript", wifiToJSonTSend.c_str());
    Serial.println("wifis sent");


  });
  
    server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
      Serial.println("+++++ POST pour tout changer +++++++");


      
      iteNoWifiActionCheck=0; //pour eviter une tentative de reconnection
        previousNoWifiActionCheck = millis();
      //if(.compareTo(editingPassword)== 0)
      preferences.begin(prefNameSpace.c_str(), false);

               
      int params = request->params();
      for(int i=0;i<params;i++){
        AsyncWebParameter* p = request->getParam(i);
        if(p->isPost()){
            Serial.print("     verifier: ");
            Serial.println(p->name());
          
          // HTTP POST ssid value
          if (p->name() == PARAM_INPUT_1) {
            if(request->getParam("updateWifiToo")->value() == "1" || request->getParam("updateWifiToo")->value() == "true")
            {
              ssid = p->value().c_str();
              Serial.print("SSID set to: ");
              Serial.println(ssid);
              // Write file to save value
              //writeFile(LittleFS, ssidPath, ssid.c_str());
              preferences.putString("ssid", ssid.c_str()); 
            }
          }
          // HTTP POST pass value
          if (p->name() == PARAM_INPUT_2) {
            if(request->getParam("updateWifiToo")->value() == "1" || request->getParam("updateWifiToo")->value() == "true")
            {
              wifipass = p->value().c_str();
              Serial.print("wifi Password set to: ");
              Serial.println(wifipass);
              // Write file to save value
              //writeFile(LittleFS, wifipassPath, wifipass.c_str());
              preferences.putString("wifipass", wifipass.c_str());
            }
          }
          if (p->name() == PARAM_INPUT_11) {
          
            if(request->getParam("updateWifiToo")->value() == "1" || request->getParam("updateWifiToo")->value() == "true")
            {
              wifimulti = new bool(p->value().c_str());
              Serial.print("wifimulti set to: ");
              Serial.println(wifimulti);
              // Write file to save value
              //writeFile(LittleFS, wifipassPath, wifipass.c_str());
              preferences.putBool("wifimulti", wifimulti);
            }
          }
          
          // HTTP POST ip value
//          if (p->name() == PARAM_INPUT_3) {
//            ip = p->value().c_str();
//            Serial.print("IP Address set to: ");
//            Serial.println(ip);
//            // Write file to save value
//            writeFile(LittleFS, ipPath, ip.c_str());
//          }
          // HTTP POST gateway value
//          if (p->name() == PARAM_INPUT_4) {
//            gateway = p->value().c_str();
//            Serial.print("Gateway set to: ");
//            Serial.println(gateway);
//            // Write file to save value
//            writeFile(LittleFS, gatewayPath, gateway.c_str());
//          }
          // HTTP POST hostname value
          if (p->name() == PARAM_INPUT_5) {
            hostname = p->value().c_str();
            Serial.print("hostname set to: ");
            Serial.println(hostname);
            // Write file to save value
            //writeFile(LittleFS, hostnamePath, hostname.c_str());
            preferences.putString("hostname", hostname.c_str());
          }
          // HTTP POST login value
          if (p->name() == PARAM_INPUT_6) {
            login = p->value().c_str();
            Serial.print("login set to: ");
            Serial.println(login);
            // Write file to save value
            //writeFile(LittleFS, loginPath, login.c_str());
            preferences.putString("login", login.c_str());
                
          }
          // HTTP POST pwd value
          if (p->name() == PARAM_INPUT_7) {
            pwd = p->value().c_str();
//            if(strlen(pwd.c_str()) == 0)
//            {
//              Serial.print("no change in pwd as pwd param is empty ");
//            }
//            else{
              Serial.print("pwd set to: ");
              Serial.println(pwd);
              // Write file to save value
              //writeFile(LittleFS, pwdPath, pwd.c_str());
              preferences.putString("password", pwd.c_str());
//            }
          }
//          if (p->name() == PARAM_INPUT_17) {
//            pwd = "";
//          
//            Serial.print("pwd set to [");
//            Serial.print(pwd);
//            Serial.println("] (nothing) as the no password box is checked");
//            // Write file to save value
//            //writeFile(LittleFS, pwdPath, pwd.c_str());
//            preferences.putString("password", pwd.c_str());
//          
//          }
          if (p->name() == PARAM_INPUT_8) {
            withSocketFallToWifi = ((p->value().toInt()==1)?true:false);
            Serial.print("withSocketFallToWifi set to: ");
            Serial.println(withSocketFallToWifi);
            // Write file to save value
            //writeFile(LittleFS, pwdPath, pwd.c_str());
            preferences.putBool("withSocketFallToWifi", withSocketFallToWifi);
          }
          if (p->name() == PARAM_INPUT_9) {
            myId = p->value().c_str();
            Serial.print("myId set to: ");
            Serial.println(myId);
            // Write file to save value
            //writeFile(LittleFS, pwdPath, pwd.c_str());
            preferences.putString("MY_ID", myId);
          }
          if (p->name() == PARAM_INPUT_10) {
            gpioLed = p->value().toInt();//as<int>();
            Serial.print("changement GPIOLED pour : ");
            Serial.println(gpioLed);
            // Write file to save value
            //writeFile(LittleFS, pwdPath, pwd.c_str());
            preferences.putInt("gpioLed", gpioLed);
            preferences.putString("gpioLedStr", (new String(gpioLed))->c_str());
          }
          
          if (p->name() == PARAM_INPUT_15) {
            useEth = (p->value() == "1" || p->value() == "true");
            Serial.print("changement useEth pour : ");
            Serial.println(useEth);
            // Write file to save value
            //writeFile(LittleFS, pwdPath, pwd.c_str());
            preferences.putBool("useEth", useEth);
            
          }
           if (p->name() == PARAM_INPUT_16) {
            myPhy_addr = (p->value() == "1" || p->value() == "true");
            Serial.print("changement myPhy_addr pour : ");
            Serial.println(myPhy_addr);
            // Write file to save value
            //writeFile(LittleFS, pwdPath, pwd.c_str());
            preferences.putInt("myPhy_addr", myPhy_addr);
            
          }
          if (p->name() == PARAM_INPUT_23) {
            Serial.print("changement isWt32 par serveur: valeur de p: ");
            Serial.println(p->value());
            //isWt32 = (p->value() == "1" || p->value() == "true");
            isWt32 = new bool(p->value() );

            Serial.print("changement isWt32 pour : ");
            Serial.println(isWt32);
            // Write file to save value
            //writeFile(LittleFS, pwdPath, pwd.c_str());
            preferences.putBool("isWt32", isWt32);
            
          }

          //Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
        }
      }
      onForXminBool = false;
      makeAPOn = false;
       Serial.print("makeAPOn set to: ");
       Serial.println(makeAPOn);
      preferences.putBool("makeAPOn", makeAPOn);
      makeAPOn = preferences.getBool("makeAPOn", false);
      Serial.print("makeAPOn depuis les prefs: ");
      Serial.println(makeAPOn);
      
      loadPortier = true;
       Serial.print("loadPortier set to: ");
       Serial.println(loadPortier);
      preferences.putBool("loadPortier", loadPortier);
      loadPortier = preferences.getBool("loadPortier", false);
      Serial.print("loadPortier depuis les prefs: ");
      Serial.println(loadPortier);

      
      preferences.end();
          
      //delay(500);
      //loadFromWeb();  //je ne peux pas faire de loadfromweb ici car je ne suis pas connecte au reseau!!!!  1/2 journee pour cela...
      setupByConfig();
      Serial.print("ssid: ");
      Serial.println(ssid);
      Serial.print("wifpass: ");
      Serial.println(wifipass);
      Serial.print("hostname: ");
      Serial.println(hostname);
      Serial.print("socketHostname: ");
      Serial.println(socketHostname);
      Serial.print("port: ");
      Serial.println(hostnamePort);
      Serial.print("login: ");
      Serial.println(login);
      Serial.print("pwd: ");
      Serial.println(pwd);
      Serial.print("makeAPOn: ");
      Serial.println(makeAPOn);
      Serial.print("loadPortier: ");
      Serial.println(loadPortier);
      Serial.print("myId: ");
      Serial.println(myId);
      Serial.print("gpioLed: ");
      Serial.println(gpioLed);
      Serial.print("editingPasword: ");
      Serial.println(editingPassword);
     
      //request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + ip);
      request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: find it yourself" );
      delay(2000);
      ESP.restart();
    });

    server.on("/restart", HTTP_GET, [](AsyncWebServerRequest *request) {
      Serial.println("server:/restart call");
      preferences.begin(prefNameSpace.c_str(), false);
        onForXminBool = false;
        makeAPOn = false;
         Serial.print("makeAPOn set to: ");
         Serial.println(makeAPOn);
        preferences.putBool("makeAPOn", makeAPOn);
        maintenanceOn = false;
         Serial.print("maintenanceOn set to: ");
         Serial.println(maintenanceOn);
        preferences.putBool("maintenanceOn", maintenanceOn);
      preferences.end();
      Serial.println("restarting");
      request->send(200, "text/plain", "restarting: going to try wifi and then to connect to the server.  if not, will get back to access point." );
      delay(500);
      ESP.restart();
    });
   
    server.on("/checkPassword", HTTP_GET, [](AsyncWebServerRequest *request) {
      Serial.println("server:/checkPassword call");
      bool result = false;
      String resultTxt = "";
      int statusCode = 200;
      //List all collected headers (Compatibility)
      int headers = request->headers();
      int i;
      for(i=0;i<headers;i++){
        Serial.printf("HEADER[%s]: %s\n", request->headerName(i).c_str(), request->header(i).c_str());
      }
      
      //get specific header by name
      if(request->hasHeader("Authorization")){
        AsyncWebHeader* h = request->getHeader("Authorization");
        Serial.printf("Authorization: %s\n", h->value().c_str());        
        Serial.printf("Authorization: == %s?", editingPassword.c_str());
        //Serial.printf("Authorization: decoded: == %s?", base64_decode(editingPassword));
        if(!editingPassword.isEmpty() && !h->value().isEmpty())
          Serial.println((h->value().compareTo(editingPassword) == 0));
        else
          Serial.printf("editing password [%s] ou h->value [%s] sont vides, on ne compare pas\n", editingPassword.c_str(),h->value().c_str());
        

        String pwdFound = splitBearer(h->value()," ");
        Serial.printf("Authorization: %s == %s?",pwdFound, editingPassword.c_str());
        if(!editingPassword.isEmpty() && !pwdFound.isEmpty())
          Serial.println((pwdFound.compareTo(editingPassword) == 0));
        else
          Serial.printf("editing password [%s] ou pwdFound [%s] sont vides, on ne compare pas\n", editingPassword.c_str(),pwdFound.c_str());



        if(!editingPassword.isEmpty() && !h->value().isEmpty() && h->value().compareTo(editingPassword) == 0){
          Serial.printf("Authorization: %s\n", h->value().c_str());
          result = true;
        }else if(!editingPassword.isEmpty() && !pwdFound.isEmpty() && pwdFound.compareTo(editingPassword) == 0)
        {
          Serial.printf("Authorization: %s == %s? oui",pwdFound, editingPassword.c_str());
          Serial.println("Authorization: result a true");
          result = true;
        }else if(editingPassword.isEmpty() && !pwdFound.isEmpty())
        {
           Serial.println("Authorization: probleme potentiel, editinpassword est vide");
          resultTxt="empty editing password: potential problem";
        }
        else
        {
                   
          Serial.println("Authorization: pas de mot de passe extrait ou correct trouvé");
          resultTxt="empty password or no extracted password or compared right";
//              char *cstr = new char[h->value().length() + 1];
//              strcpy(cstr, h->value().c_str());
//              // do stuff
//              
//              char *token = strtok(cstr, "Bearer ");
//   
//              // Keep printing tokens while one of the
//              // delimiters present in str[].
//              while (token != NULL)
//              {
//                  //printf("trouve: %s\n", token);
//                  printf("trouve: %s\n", token);
//                  Serial.printf("Authorization: %s == %s?",String(token).c_str(), editingPassword.c_str());
//                  Serial.println((String(token).compareTo(editingPassword) == 0));
//                  if(String(token).compareTo(editingPassword) == 0)
//                  {
//                    token = NULL;
//                    result = true;
//                  }
//                  
//              }
//              delete [] cstr;
        }
      }
      String resultString = (result?"true":"false");
      String toSend = "{\"result\":\""+resultString+"\",\"resultTxt\":\""+resultTxt+"\",\"delayBeforeApDown\":\""+(onForXminBool?String(XminDelay):String(intervalNoWifiActionCheck*maxIteNoWifiActionBeforeReboot))+"\"}";
      Serial.printf("Authorization: sending code(%s) : %s \n",String(statusCode).c_str(), toSend.c_str());
      Serial.print("Authorization: running on core: ");
      Serial.println(xPortGetCoreID());

      // truc chiant pour affectation memoire
      char* str;

         /* Calcul de la taille à allouer */
      size_t l = strlen( toSend.c_str() ) + 1;
  
      /* Allocation de la mémoire */
      str = (char *) malloc( l * sizeof(char) );
      assert( str != NULL );
      //Serial.println("[splitBearer] str est pourri, on tente de lui affecter qqch de la taille du hostname puis on le copie ");
  
      //*str = malloc(_hostname.length);
      strcpy(str, toSend.c_str());
  
      // --- truc chiant pour affectation memoire
      request->send(statusCode, "application/javascript", str );

      
      //const char response_char[] PROGMEM = "{\"result\":\""+resultString+"\",\"resultTxt\":\""+resultTxt+"\",\"delayBeforeApDown\":\""+(onForXminBool?String(XminDelay):String(intervalNoWifiActionCheck*maxIteNoWifiActionBeforeReboot))+"\"}";
      //request->send(statusCode, "application/javascript", response_char );
      
    });
    server.on("/getpassword795WWjidhzvjio847756298dyyop11098c7yfhhu9w9huapm1vnxbxznniweuyrhb53tiwiuyaksjdfuizx3cvgfkopRdfdPIkgti43enkrewhui99aopqit78943",HTTP_GET, [](AsyncWebServerRequest *request) {
      int statusCode = 404;
      String str = "";
      if(editingPassword.isEmpty())
      {
        statusCode = 201;
        str = "emptyPassword";
      }else
      {
        statusCode = 200;
        str = editingPassword;
      }

      request->send(statusCode, "application/javascript", str);      

    });
    server.on("/setpassword", HTTP_POST, [](AsyncWebServerRequest *request) {
      //if(.compareTo(editingPassword)== 0)
      Serial.println("POST /setpassword");
      int statusCode = 404;
      iteNoWifiActionCheck=0;
      previousNoWifiActionCheck = millis();

      bool result = false;
      String resultTxt = "";
      bool canGoOn = false;
      String pwdFound = "";

      if(request->hasHeader("Authorization")){
        AsyncWebHeader* h = request->getHeader("Authorization");
        Serial.printf("/setpassword: Authorization: [%s] == [%s]? \n", h->value().c_str(), editingPassword.c_str());
        //Serial.printf("Authorization: decoded: == %s?", base64_decode(editingPassword));
        if(!editingPassword.isEmpty() && !h->value().isEmpty())
          Serial.println((h->value().compareTo(editingPassword) == 0));
        else
          Serial.println("\n/setpassword: editing password ou h->value sont vides, on ne compare pas");

        pwdFound = splitBearer(h->value()," ");
      }
      
      Serial.printf("/setpassword: editing password [%s] is empty? ",editingPassword.c_str());
      Serial.println(editingPassword.isEmpty());
      Serial.printf("/setpassword: pwdFound [%s] is empty? ",pwdFound.c_str());
      Serial.println(pwdFound.isEmpty());
      if( (editingPassword.isEmpty() || ( !pwdFound.isEmpty() && pwdFound.compareTo(editingPassword) == 0) ) ) // || (editingPassword.compareTo("") == 0) )
      {
        if(request->hasParam("newPwd", true))
        {
          String newPwd = request->getParam("newPwd", true)->value();
          Serial.printf("/setpassword: new pwd found: == [%s] \n", newPwd.c_str());

          preferences.begin(prefNameSpace.c_str(), false);
            preferences.putString("editingPassword", newPwd);
            editingPassword =  preferences.getString("editingPassword", "");
          preferences.end();
  
          statusCode = 200;
          result = true;
        }else{
          statusCode = 400;
          resultTxt = "no newPwd found";
        }

      }
      else
      {
        statusCode = 401;
        resultTxt = "wrong initial password";
      }
      
      //list all parameters
       Serial.println("list params: +++++");

      int params = request->params(); 
      for(int i=0;i<params;i++){
        AsyncWebParameter* p = request->getParam(i);
        if(p->isFile()){ //p->isPost() is also true
          Serial.printf("FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
        } else if(p->isPost()){
          Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
        } else {
          Serial.printf("GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
        }
      }
     Serial.println("list params -----");


      String resultString = (result?"true":"false");
      String toSend = "{\"result\":\""+resultString+"\",\"resultTxt\":\""+resultTxt+"\",\"delayBeforeApDown\":\""+(onForXminBool?String(XminDelay):String(intervalNoWifiActionCheck*maxIteNoWifiActionBeforeReboot))+"\"}";
      Serial.printf("/setpassword: sending code(%s) : %s",String(statusCode).c_str(), toSend.c_str());
      Serial.println();
      Serial.print("setpassword: running on core: ");
      Serial.println(xPortGetCoreID());


      // truc chiant pour affectation memoire
      char* str;

         /* Calcul de la taille à allouer */
      size_t l = strlen( toSend.c_str() ) + 1;
  
      /* Allocation de la mémoire */
      str = (char *) malloc( l * sizeof(char) );
      assert( str != NULL );
  
      //*str = malloc(_hostname.length);
      strcpy(str, toSend.c_str());
  
      // --- truc chiant pour affectation memoire

      
      request->send(statusCode, "application/javascript", str);      
      //const char response_char[] PROGMEM = "{\"result\":\""+resultString+"\",\"resultTxt\":\""+resultTxt+"\",\"delayBeforeApDown\":\""+(onForXminBool?String(XminDelay):String(intervalNoWifiActionCheck*maxIteNoWifiActionBeforeReboot))+"\"}";
      //request->send(statusCode, "application/javascript", response_char );      
      //request->send(statusCode, "application/javascript", toSend.c_str() );      
      
    });


 server.on("/updateStaticIp", HTTP_POST, [](AsyncWebServerRequest *request) {
      //if(.compareTo(editingPassword)== 0)
      Serial.println("POST pour static IP");
      iteNoWifiActionCheck=0;
      previousNoWifiActionCheck = millis();

    preferences.begin(prefNameSpace.c_str(), false);
    int params = request->params();
      for(int i=0;i<params;i++){
        AsyncWebParameter* p = request->getParam(i);
        Serial.printf("parametre [%d] : ", i);
        //Serial.println(p->name());
        if(p->isPost()){
          Serial.println(p->name());
        
//          // HTTP POST ssid value
//          if (p->name() == PARAM_INPUT_1) {
//            ssid = p->value().c_str();
//            Serial.print("SSID set to: ");
//            Serial.println(ssid);
//            // Write file to save value
//            //writeFile(LittleFS, ssidPath, ssid.c_str());
//            preferences.putString("ssid", ssid.c_str()); 
//
//          }
//          // HTTP POST pass value
//          if (p->name() == PARAM_INPUT_2) {
//            wifipass = p->value().c_str();
//            Serial.print("wifi Password set to: ");
//            Serial.println(wifipass);
//            // Write file to save value
//            //writeFile(LittleFS, wifipassPath, wifipass.c_str());
//            preferences.putString("wifipass", wifipass.c_str());
//          }
//
//          if (p->name() == PARAM_INPUT_11) {
//            wifimulti = new bool(p->value().c_str());
//            Serial.print("wifimulti set to: ");
//            Serial.println(wifimulti);
//            // Write file to save value
//            //writeFile(LittleFS, wifipassPath, wifipass.c_str());
//            preferences.putBool("wifimulti", wifimulti);
//          }
              // HTTP POST ip value

          
          if (p->name() == PARAM_INPUT_14) {
            useStaticIp = (p->value().indexOf("true") == 0 || p->value().indexOf("1") == 0);
            Serial.print("useStaticIp devrait set to: ");
            Serial.println(p->value().c_str());
            Serial.print("useStaticIp set to: ");
            Serial.println(useStaticIp);
            // Write file to save value
            preferences.putBool("useStaticIp", useStaticIp);
          }

          if (p->name() == PARAM_INPUT_3) {
            ip = p->value().c_str();
            Serial.print("IP Address set to: ");
            Serial.println(ip);
            // Write file to save value
            preferences.putString("ip", ip);
          }
           // HTTP POST gateway value
          if (p->name() == PARAM_INPUT_4) {
            gateway = p->value().c_str();
            Serial.print("Gateway set to: ");
            Serial.println(gateway);
            // Write file to save value
            preferences.putString("gateway", gateway.c_str());
          }
          // HTTP POST dns value
          if (p->name() == PARAM_INPUT_12) {
            dns = p->value().c_str();
            Serial.print("dns set to: ");
            Serial.println(dns);
            // Write file to save value
            preferences.putString("dns", dns.c_str());
          }
          // HTTP POST mask value
          if (p->name() == PARAM_INPUT_13) {
            mask = p->value().c_str();
            Serial.print("mask set to: ");
            Serial.println(mask);
            // Write file to save value
            preferences.putString("mask", mask.c_str());
          }

           if (p->name() == PARAM_INPUT_18) {
            useStaticIp_eth = (p->value().indexOf("true") == 0 || p->value().indexOf("1") == 0);
            Serial.print("useStaticIp_eth devrait set to: ");
            Serial.println(p->value().c_str());
            Serial.print("useStaticIp_eth set to: ");
            Serial.println(useStaticIp_eth);
            // Write file to save value
            preferences.putBool("useStaticIp_eth", useStaticIp_eth);
          }

          if (p->name() == PARAM_INPUT_19) {
            ip_eth = p->value().c_str();
            Serial.print("IP_eth Address set to: ");
            Serial.println(ip_eth);
            // Write file to save value
            preferences.putString("ip_eth", ip_eth);
          }
           // HTTP POST gateway value
          if (p->name() == PARAM_INPUT_20) {
            gateway_eth = p->value().c_str();
            Serial.print("Gateway_eth set to: ");
            Serial.println(gateway_eth);
            // Write file to save value
            preferences.putString("gateway_eth", gateway_eth.c_str());
          }
          // HTTP POST dns value
          if (p->name() == PARAM_INPUT_21) {
            dns_eth = p->value().c_str();
            Serial.print("dns_eth set to: ");
            Serial.println(dns_eth);
            // Write file to save value
            preferences.putString("dns_eth", dns_eth.c_str());
          }
          // HTTP POST mask value
          if (p->name() == PARAM_INPUT_22) {
            mask_eth = p->value().c_str();
            Serial.print("mask_eth set to: ");
            Serial.println(mask_eth);
            // Write file to save value
            preferences.putString("mask_eth", mask_eth.c_str());
          }
          // HTTP POST isWt32 value
          if (p->name() == PARAM_INPUT_23) {
            isWt32 = new bool(p->value().c_str());
            Serial.print("isWt32set to: ");
            Serial.println(isWt32);
            // Write file to save value
            preferences.putBool("isWt32", isWt32);
          }

          else{
            Serial.print("variable [");Serial.print(i);Serial.print("]: ");Serial.print(p->name());Serial.print(": ");
            Serial.println(p->value().c_str());
          }
        }
      }
      
      Serial.print("updateStaticIp: has param restartAfterStaticIp: [");
      Serial.print(request->hasParam("restartAfterStaticIp", true));
      Serial.println("]");

      if(false) //request->hasParam("restartAfterStaticIp", true))
      {
            Serial.print("updateStaticip: has param restartAfterWifi: [true] :name:");
            Serial.println(request->getParam("restartAfterWifi",true)->name());
            Serial.print("updateStaticIp: has param restartAfterWifi: [true] :getValue:");
            Serial.println(request->getParam("restartAfterWifi",true)->value());
            Serial.print("updateStaticIp: has param restartAfterWifi: [true] : == '1'?:");
            Serial.print(request->getParam("restartAfterWifi",true)->value().compareTo("1") == 0 );
            Serial.print(" ou le compare to donne: ");
            Serial.println(request->getParam("restartAfterWifi",true)->value().compareTo("1"));
            //if(request->getParam("restartAfterWifi")->value())  
            if(false) //request->getParam("restartAfterStaticIp",true)->value().compareTo("on") == 0 || request->getParam("restartAfterStaticIp",true)->value().compareTo("1") == 0 || request->getParam("restartAfterWifi",true)->value().compareTo("true") == 0)
            {
              Serial.print("updateStaticIp: has param restartAfterWifi: GOING TO RESTART");
              
                makeAPOn = false;
                preferences.putBool("makeAPOn", makeAPOn);
                loadPortier = true;
                preferences.putBool("loadPortier", loadPortier);
                maintenanceOn = false;
                preferences.putBool("maintenanceOn", maintenanceOn);
                preferences.end();
                //request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + ip);
                request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: find it yourself" );
          
                delay(3000);
                ESP.restart();
            }
            else
            {
              iteNoWifiActionCheck=0;
              previousNoWifiActionCheck = millis();
              preferences.end();
              Serial.print("updateStaticIp: has param restartAfterWifi: update then stay");
              String htmlResp="<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"></meta></head><body><span>Update updateStaticIp done.</span><br><span><form  action=\"/\" method=\"GET\" style=\"background-color: blue\">  <input type =\"submit\" value =\"back\"></input></form></span></body></html>";
              request->send(200, "text/html", htmlResp);
            }
      }
      else
      {
        Serial.print("updateStaticIp: NOT has param restartAfterWifi: update then stay");
        String htmlResp="<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"></meta></head><body><span>Update static ip conf done.</span><br><span><form  action=\"/\" method=\"GET\" style=\"background-color: blue\">  <input type =\"submit\" value =\"Submit\">back</input></form></span></body></html>";
        iteNoWifiActionCheck=0;
        previousNoWifiActionCheck = millis();
        preferences.end();
        request->send(200, "text/html", htmlResp);

      }
    });

    server.on("/setwifi", HTTP_POST, [](AsyncWebServerRequest *request) {
      //if(.compareTo(editingPassword)== 0)
      Serial.println("POST pour wifi");
      iteNoWifiActionCheck=0;
      previousNoWifiActionCheck = millis();

    preferences.begin(prefNameSpace.c_str(), false);
    int params = request->params();
      for(int i=0;i<params;i++){
        AsyncWebParameter* p = request->getParam(i);
        Serial.printf("parametre [%d]", i);
        //Serial.println(p->name());
        if(p->isPost()){
          Serial.println(p->name());
          Serial.print("variable: ");Serial.print(i);Serial.print(": ");
          Serial.println(p->value().c_str());
          
          // HTTP POST ssid value
          if (p->name() == PARAM_INPUT_1) {
            ssid = p->value().c_str();
            Serial.print("SSID set to: ");
            Serial.println(ssid);
            // Write file to save value
            //writeFile(LittleFS, ssidPath, ssid.c_str());
            preferences.putString("ssid", ssid.c_str()); 

          }
          // HTTP POST pass value
          if (p->name() == PARAM_INPUT_2) {
            wifipass = p->value().c_str();
            if(strlen(wifipass.c_str()) == 0)
            {
              Serial.print("no change in wifipass as wifipass param is empty ");
            }
            else{
              Serial.print("wifi Password set to: ");
              Serial.println(wifipass);
              // Write file to save value
              //writeFile(LittleFS, wifipassPath, wifipass.c_str());
              preferences.putString("wifipass", wifipass.c_str());
            }
          }

           if (p->name() == PARAM_INPUT_17) {
            wifipass = "";
          
            Serial.print("wifipass set to [");
            Serial.print(wifipass);
            Serial.println("] (nothing) as the no wifipass password box is checked");
            // Write file to save value
            //writeFile(LittleFS, pwdPath, pwd.c_str());
            preferences.putString("wifipass", wifipass.c_str());
          }

           if (p->name() == PARAM_INPUT_11) {
            wifimulti = new bool(p->value().c_str());
            Serial.print("wifimulti set to: ");
            Serial.println(wifimulti);
            // Write file to save value
            //writeFile(LittleFS, wifipassPath, wifipass.c_str());
            preferences.putBool("wifimulti", wifimulti);
          }
           if (p->name() == PARAM_INPUT_15) {
            useEth = (p->value() == "1" || p->value() == "true");
            Serial.print("changement useEth pour : ");
            Serial.println(useEth);
            // Write file to save value
            //writeFile(LittleFS, pwdPath, pwd.c_str());
            preferences.putBool("useEth", useEth);
            
          }
           if (p->name() == PARAM_INPUT_16) {
            myPhy_addr = (p->value() == "1" || p->value() == "true");
            Serial.print("changement myPhy_addr pour : ");
            Serial.println(myPhy_addr);
            // Write file to save value
            //writeFile(LittleFS, pwdPath, pwd.c_str());
            preferences.putInt("myPhy_addr", myPhy_addr);
            
          }
          if (p->name() == PARAM_INPUT_23) {
            Serial.print("changement isWt32: valeur de p: ");
            Serial.println(p->value());
            //isWt32 = new bool(p->value() );
            
            Serial.print("changement isWt32  avant [");
            Serial.print(isWt32);
            Serial.print("] apres :");
            isWt32 = (p->value() == "1" || p->value() == "true");
            Serial.println(isWt32);
            // Write file to save value
            //writeFile(LittleFS, pwdPath, pwd.c_str());
            preferences.putBool("isWt32", isWt32);
            
          }
//          else{
//            Serial.print("changement isWt32: mise a faux car n'est pas dans les param: ");
//            Serial.println(p->value());
//            //isWt32 = (p->value() == "1" || p->value() == "true");
//              Serial.print("changement isWt32 pour false. avant [");
//            Serial.print(isWt32);
//            Serial.print("] apres :");
//            isWt32 = false;
//            Serial.println(isWt32);
//            // Write file to save value
//            //writeFile(LittleFS, pwdPath, pwd.c_str());
//            preferences.putBool("isWt32", isWt32);
//          }
          
          
        }
      }
      
      Serial.print("setwifi: has param restartAfterWifi: [");
      Serial.print(request->hasParam("restartAfterWifi", true));
      Serial.println("]");

      if(request->hasParam("restartAfterWifi", true))
      {
            Serial.print("setwifi: has param restartAfterWifi: [true] :name:");
            Serial.println(request->getParam("restartAfterWifi",true)->name());
            Serial.print("setwifi: has param restartAfterWifi: [true] :getValue:");
            Serial.println(request->getParam("restartAfterWifi",true)->value());
            Serial.print("setwifi: has param restartAfterWifi: [true] : == '1'?:");
            Serial.print(request->getParam("restartAfterWifi",true)->value().compareTo("1") == 0 );
            Serial.print(" ou le compare to donne: ");
            Serial.println(request->getParam("restartAfterWifi",true)->value().compareTo("1"));
            //if(request->getParam("restartAfterWifi")->value())
            if(request->getParam("restartAfterWifi",true)->value().compareTo("on") == 0 || request->getParam("restartAfterWifi",true)->value().compareTo("1") == 0 || request->getParam("restartAfterWifi",true)->value().compareTo("true") == 0)
            {
              Serial.print("setwifi: has param restartAfterWifi: GOING TO RESTART");
              
                makeAPOn = false;
                preferences.putBool("makeAPOn", makeAPOn);
                loadPortier = true;
                preferences.putBool("loadPortier", loadPortier);
                maintenanceOn = false;
                preferences.putBool("maintenanceOn", maintenanceOn);
                preferences.end();
                //request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + ip);
                request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: find it yourself" );
          
                delay(3000);
                ESP.restart();
            }
            else
            {
              iteNoWifiActionCheck=0;
              previousNoWifiActionCheck = millis();
              preferences.end();
              Serial.print("setwifi: has param restartAfterWifi: update then stay");
              String htmlResp="<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"></meta></head><body><span>Update wifi done.</span><br><span><form  action=\"/\" method=\"GET\" style=\"background-color: blue\">  <input type =\"submit\" value =\"back\"></input></form></span></body></html>";
              request->send(200, "text/html", htmlResp);
            }
      }
      else
      {
        //Serial.print("setwifi: has NOT param restartAfterWifi: update then restart");
        Serial.print("setwifi: NOT has param restartAfterWifi: update then stay");
        String htmlResp="<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"></meta></head><body><span>Update wifi done.</span><br><span><form  action=\"/\" method=\"GET\" style=\"background-color: blue\">  <input type =\"submit\" value =\"Submit\">back</input></form></span></body></html>";
        iteNoWifiActionCheck=0;
        previousNoWifiActionCheck = millis();
        preferences.end();
        request->send(200, "text/html", htmlResp);
//        makeAPOn = false;
//        preferences.putBool("makeAPOn", makeAPOn);
//        loadPortier = true;
//        preferences.putBool("loadPortier", loadPortier);
//        maintenanceOn = false;
//        preferences.putBool("maintenanceOn", maintenanceOn);
//        preferences.end();
//        //request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + ip);
//        request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: find it yourself" );
//  
//        delay(3000);
//        ESP.restart();
      }
    });
    Serial.println("server ready");
    AsyncElegantOTA.begin(&server);
    server.begin();
    Serial.println("serveur AP demarré");
    Serial.print("ssid [");Serial.print(ssid);Serial.println("], si =='' alors l'Ap va rester ON");

    Serial.println("+-+-+-+-+-++-+-");
    }

  Serial.printf("ESP32 Chip model = %s Rev %d\n", ESP.getChipModel(), ESP.getChipRevision());
  Serial.printf("This chip has %d cores\n", ESP.getChipCores());
  Serial.printf("This chip mac address: %d \n", ESP.getEfuseMac());
  Serial.printf("%012llx", ESP.getEfuseMac()); Serial.println();
  Serial.print("\n Board: " + String(ARDUINO_BOARD));
  Serial.println(" shield: " + String(SHIELD_TYPE));
 
  //uint8_t *MAC = ESP.getEfuseMac();
  //Serial.printf("%02x%02x%02x%02x%02x%02x\n", MAC[5], MAC[4], MAC[3], MAC[2], MAC[1], MAC[0]);
  
  

}


void loop() {
    socketIO.loop();
    if(WiFi.getMode() == WIFI_AP && (ssid != "" || (isWt32 && useEth )) )
      checkNoWifiActionDone();
//    if(!makeAPOn && (WiFi.getMode() == WIFI_STA))
//      checkSocketConnected(socketIO);
//    //else mettre un delay avant de retenter la connection wifi?
    
}
