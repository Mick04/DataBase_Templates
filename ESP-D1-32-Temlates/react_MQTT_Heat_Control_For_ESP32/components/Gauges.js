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
  const [ESP32outSide, setOutSideTemp] = useState("");
  const [ESP32coolSide, setCoolSideTemp] = useState("");
  const [ESP32heater, setHeaterTemp] = useState("");
  const [ESP32gaugeHours, setGaugeHours] = useState(0);
  const [ESP32gaugeMinutes, setGaugeMinutes] = useState(0);
  const [HeaterStatus, setHeaterStatus] = useState(false);
  const [ESP32targetTemperature, setTargetTemperature] = useState(0);
  const [isConnected, setIsConnected] = useState(false);

  // Define the onMessageArrived callback
  const onMessageArrived = useCallback((message) => {
    switch (message.destinationName) {
      case "ESP32outSide":
        setOutSideTemp(parseFloat(message.payloadString).toFixed(1));
        break;
      case "ESP32coolSide":
        setCoolSideTemp(parseFloat(message.payloadString).toFixed(1));
        break;
      case "ESP32heater":
        setHeaterTemp(parseFloat(message.payloadString).toFixed(1));
        break;
      case "ESP32gaugeHours":
        setGaugeHours(parseInt(message.payloadString));
        break;
      case "ESP32gaugeMinutes":
        setGaugeMinutes(parseInt(message.payloadString));
        break;
      case "HeaterStatus":
        const newStatus = message.payloadString.trim() === "true";
        setHeaterStatus(newStatus);
        break;
      case "ESP32targetTemperature":
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
      mqtt.connect("ESP32Tortiose", "Hea1951TerESP32", {
        onSuccess: () => {
          // console.log(
          //   "Settings line 76 TemperatureGraph Connected to MQTT broker"
          // );
          setIsConnected(true);
          mqtt.client.subscribe("ESP32outSide");
          mqtt.client.subscribe("ESP32coolSide");
          mqtt.client.subscribe("ESP32heater");
          mqtt.client.subscribe("ESP32gaugeHours");
          mqtt.client.subscribe("ESP32gaugeMinutes");
          mqtt.client.subscribe("HeaterStatus");
          mqtt.client.subscribe("ESP32targetTemperature");
        },
        onFailure: (error) => {
          // console.error("Failed to connect to MQTT broker", error);
          setIsConnected(false);
        },
      });

      setMqttService(mqtt);

      return () => {
        console.log("GaugeScreen is unfocused");
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
        <Text style={styles.ESPHeader}>MQTT_Heat_Control_ESP32</Text>
        <Text style={styles.heading}>Gauges</Text>
        <Text style={styles.timeHeader}>
          If time is incorrect, check housing
        </Text>
        <View>
          <Text style={styles.timeText}>Hours: Minutes</Text>
          <Text style={styles.time}>
            {ESP32gaugeHours}:{ESP32gaugeMinutes.toString().padStart(2, "0")}
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
          {"Target Temperature = " + ESP32targetTemperature}{" "}
        </Text>
        <View>
          <Text style={[styles.tempText, { color: "black" }]}>
            {"outSide Temperature = " + ESP32outSide + "\n"}
          </Text>
          <Text style={[styles.tempText, { color: "green" }]}>
            {"coolSide Temperature = " + ESP32coolSide + "\n"}
          </Text>

          <Text style={[styles.tempText, { color: "red" }]}>
            {"heater Temperature = " + ESP32heater}
          </Text>
        </View>
        <View>
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
