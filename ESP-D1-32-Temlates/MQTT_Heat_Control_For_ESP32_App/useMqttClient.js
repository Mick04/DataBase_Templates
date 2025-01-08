// Importing necessary libraries and components
import Paho from "paho-mqtt";
import { useEffect, useState } from "react";

/************************************
 *    Creating a new MQTT client    *
 *              start               *
 * **********************************/

const client = new Paho.Client(
  "public.mqtthq.com",
  Number(1883),
  `inTopic-${parseInt(Math.random() * 100)}`
);

/************************************
 *    Creating a new MQTT client    *
 *                end               *
 * **********************************/

/************************************
 *          Main component          *
 *              start               *
 * **********************************/

const useMqttClient = () => {
  /************************************
   *          State variable          *
   *              start               *
   * **********************************/
  const [value, setValue] = useState(0);
  // // const [messageFromWemos, setMessageFromWemos] = useState("");
  const [isConnected, setIsConnected] = useState(false);

  /************************************
   *          State variable          *
   *                end               *
   * **********************************/

  /********************************************************************
   *   Effect hook to establish MQTT connection and handle messages   *
   *                          start                                   *
   * ******************************************************************/
  useEffect(() => {
    // Function to handle successful connection
    function onConnect() {
      console.log("Connected!");
      setIsConnected(true);
      client.subscribe("topic1"); // Subscribe to 'topic1'
      client.subscribe("topic2"); // Subscribe to 'topic2'
      client.subscribe("topic3"); // Subscribe to 'topic3'
      client.subscribe("topic4"); // Subscribe to 'topic4'
      client.subscribe("topic5"); // Subscribe to 'topic5'
    }
    // Function to handle connection failure
    function onFailure() {
      console.log("Failed to connect!");
      setIsConnected(false);
    }
    /***********************************************************************
     *   Effect hook to establish MQTT connection and handle messages      *
     *                            end                                      *
     * *********************************************************************/
    /***********************************************
     *    Function to handle incoming messages     *
     *                   start                     *
     * *********************************************/
    function onMessageReceived(message) {
      console.log("Message received:", message.payloadString);
      if (message.destinationName === "topic1") {
        setValue(parseInt(message.payloadString));
      } else if (
        message.destinationName === "topic4" &&
        parseInt(message.payloadString) === 3
      ) {
        // setMessageFromWemos("Switch has been open for more than 10 minutes!");
      }
    }

    /***********************************************
     *    Function to handle incoming messages     *
     *                     end                     *
     * *********************************************/
    /***********************************************
     *          Connect to the MQTT broker         *
     *                   start                     *
     * *********************************************/
    client.connect({
      onSuccess: onConnect,
      onFailure: onFailure,
    });

    /***********************************************
     *          Connect to the MQTT broker         *
     *                     end                     *
     * *********************************************/

    /********************************************
     *           Set the message handler           *
     *                    start                    *
     * *********************************************/

    client.onMessageArrived = onMessageReceived;
    /***********************************************
     *           Set the message handler           *
     *                    end                      *
     * *********************************************/

    /*************************************************************
     *   Cleanup function to disconnect when component unmounts  *
     *                         start                             *
     * ***********************************************************/

    return () => {
      client.disconnect();
    };
  }, []);
  /*************************************************************
   *   Cleanup function to disconnect when component unmounts  *
   *                            end                            *
   * ***********************************************************/

  /*************************************************************
   *         Function to change the value and send it          *
   *                          start                            *
   * ***********************************************************/

  //   function changeValue() {
  //     if (!isConnected) {
  //       console.log("Client is not connected.");
  //       return;
  //     }
  // console.log("isConnected: ", isConnected);

  //     setValue(1);
  //     console.log(typeof "1"); // should log 'string'
  //     let message = new Paho.Message("1");
  //     console.log(message instanceof Paho.Message); // should log true
  //     message.destinationName = "inTopic";
  //     console.log(typeof message); // should log 'object'
  //     client.send(message);
  //     console.log("Message sent:", message.payloadString);

  //     //Reset the value after 5 seconds and set the value back to 0
  //     setTimeout(() => {
  //       setValue(0);
  //       message = new Paho.Message("0");
  //       message.destinationName = "inTopic";
  //       client.send(message);
  //       console.log("Message sent:", message.payloadString);
  //     }, 5000);
  //   }
  /**************************************************************
   *         Function to change the value and send it           *
   *                           end                              *
   * ***********************************************************/

  return client;
};

export default useMqttClient;
