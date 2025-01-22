//Gauges.js
import React, { useEffect, useState, useCallback } from "react";
import {
  View,
  Text,
  TouchableOpacity,
  SafeAreaView,
  ScrollView,
} from "react-native";
import { useFocusEffect } from "@react-navigation/native";
import MqttService from "./MqttService";
import { styles } from "../Styles/styles";
const GaugeScreen = () => {
  const [mqttService, setMqttService] = useState(null);
  const [outSide, setOutSideTemp] = useState("");
  const [coolSide, setCoolSideTemp] = useState("");
  const [heater, setHeaterTemp] = useState("");
  const [gaugeHours, setGaugeHours] = useState(0);
  const [gaugeMinutes, setGaugeMinutes] = useState(0);
  const [HeaterStatus, setHeaterStatus] = useState(false);
  const [targetTemperature, setTargetTemperature] = useState(0);
  const [isConnected, setIsConnected] = useState(false);

  // Define the onMessageArrived callback
  const onMessageArrived = useCallback((message) => {
    switch (message.destinationName) {
      case "outSide":
        setOutSideTemp(parseFloat(message.payloadString).toFixed(1));
        break;
      case "coolSide":
        setCoolSideTemp(parseFloat(message.payloadString).toFixed(1));
        break;
      case "heater":
        setHeaterTemp(parseFloat(message.payloadString).toFixed(1));
        break;
      case "gaugeHours":
        setGaugeHours(parseInt(message.payloadString));
        break;
      case "gaugeMinutes":
        setGaugeMinutes(parseInt(message.payloadString));
        break;
      case "HeaterStatus":
        const newStatus = message.payloadString.trim() === "true";
        setHeaterStatus(newStatus);
        break;
      case "TargetTemperature":
        setTargetTemperature(parseInt(message.payloadString));
        break;
      default:
        // console.log("Unknown topic:", message.destinationName);
    }
  }, []);

  useFocusEffect(
    useCallback(() => {
      console.log("GaugeScreen is focused");

      // Initialize the MQTT service
      const mqtt = new MqttService(onMessageArrived, setIsConnected);
      // console.log("line 55 TemperatureGraph ");
      mqtt.connect("Tortoise", "Hea1951Ter", {
        onSuccess: () => {
          // console.log(
          //   "Settings line 76 TemperatureGraph Connected to MQTT broker"
          // );
          setIsConnected(true);
          mqtt.client.subscribe("outSide");
          mqtt.client.subscribe("coolSide");
          mqtt.client.subscribe("heater");
          mqtt.client.subscribe("gaugeHours");
          mqtt.client.subscribe("gaugeMinutes");
          mqtt.client.subscribe("HeaterStatus");
          mqtt.client.subscribe("TargetTemperature");
        },
        onFailure: (error) => {
          // console.error("Failed to connect to MQTT broker", error);
          setIsConnected(false);
        },
      });

      setMqttService(mqtt);

      return () => {
        console.log("GaugeScreen is unfocused, cleaning up...");
        // Disconnect MQTT when the screen is unfocused
        if (mqtt) {
          // console.log("Gauges line 97 Disconnecting MQTT");
          mqtt.disconnect();
        }
        setIsConnected(false); // Reset connection state
      };
    }, [onMessageArrived])
  );

  function handleReconnect() {
    // console.log("Gauges line 104 Reconnecting...");
    if (mqttService) {
      mqttService.reconnect();
      mqttService.reconnectAttempts = 0;
    } else {
      // console.log("Gauges line 110 MQTT Service is not initialized");
    }
  }
  return (
    <ScrollView contentContainerStyle={styles.container}>
      <SafeAreaView style={styles.container}>
        <Text style={styles.heading}>MQTT_Heat_Control</Text>
        <Text style={styles.heading}>Gauges</Text>
        <Text style={styles.timeHeader}>
          If time is incorrect, check housing
        </Text>
        <View>
          <Text style={styles.timeText}>Hours: Minutes</Text>
          <Text style={styles.time}>
            {gaugeHours}:{gaugeMinutes.toString().padStart(2, "0")}
          </Text>
          <Text
            style={[
              styles.TargetTempText,
              { color: HeaterStatus ? "red" : "green" },
            ]}
          >
            {"Heater Status = " + (HeaterStatus ? "on" : "off")}
          </Text>
        </View>
        <Text style={styles.TargetTempText}>
          {"Target Temperature = " + targetTemperature}{" "}
        </Text>
        <View style={styles.tempContainer}>
          <Text style={[styles.tempText, { color: "black" }]}>
            {"outSide Temperature = " + outSide + "\n"}
          </Text>
          <Text style={[styles.tempText, { color: "green" }]}>
            {"coolSide Temperature = " + coolSide + "\n"}
          </Text>

          <Text style={[styles.tempText, { color: "red" }]}>
            {"heater Temperature = " + heater}
          </Text>
        </View>
        <View style={styles.connectionStatus}>
          <Text
            style={[
              styles.connectionStatus,
              { color: isConnected ? "green" : "red" },
            ]}
          >
            {isConnected
              ? "Connected to MQTT Broker"
              : "Disconnected from MQTT Broker"}
          </Text>
        </View>
        <TouchableOpacity
          style={styles.reconnectButton}
          onPress={handleReconnect}
        >
          <Text style={styles.reconnectText}>Reconnect</Text>
        </TouchableOpacity>
      </SafeAreaView>
    </ScrollView>
  );
};

export default GaugeScreen;
