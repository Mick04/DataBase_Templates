import { StatusBar } from "expo-status-bar";
import React, { useState, useEffect, useCallback } from "react";
import {
  SafeAreaView,
  TouchableOpacity,
  Text,
  View,
} from "react-native";
import { useFocusEffect } from "@react-navigation/native";
import AsyncStorage from "@react-native-async-storage/async-storage";
import MqttService from "./MqttService";
import CustomLineChart from "./CustomLineChart"; // Import the reusable component
import { styles } from "../Styles/styles";

const Heater_ON_OFF_Graph = () => {
  const [mqttService, setMqttService] = useState(null);
  const [HeaterStatus, setHeaterStatus] = useState(false);
  const [gaugeHours, setGaugeHours] = useState(0);
  const [gaugeMinutes, setGaugeMinutes] = useState(0);
  const [isConnected, setIsConnected] = useState(false);
  const [data, setData] = useState([
    // { value: 0, label: "10:00", dataPointText: "OFF" },
  ]);

  // Define the onMessageArrived callback
  const onMessageArrived = useCallback(
    (message) => {
      if (message.destinationName === "HeaterStatus") {
        const newStatus = message.payloadString.trim() === "true" ? 1 : 0;
        const lastStatus = data.length > 0 ? data[data.length - 1].value : null;
        if (lastStatus === null || newStatus !== lastStatus) {
          setHeaterStatus(newStatus);
          setData((prevData) => [
            ...prevData,
            {
              value: newStatus,
              label: new Date().toLocaleTimeString([], {
                hour: "2-digit",
                minute: "2-digit",
              }),
              dataPointText: newStatus === 1 ? "ON" : "OFF",
              textColor: newStatus === 1 ? "red" : "green",
            },
          ]);
        }
      }
      if (message.destinationName === "gaugeHours") {
        setGaugeHours(parseInt(message.payloadString));
      }
      if (message.destinationName === "gaugeMinutes") {
        setGaugeMinutes(parseInt(message.payloadString));
      }
    },
    [data, HeaterStatus]
  );
const textColor = "blue";
  useFocusEffect(
    useCallback(() => {
      console.log("GaugeScreen is focused");

      // Initialize the MQTT service
      const mqtt = new MqttService(onMessageArrived, setIsConnected);
      mqtt.connect("Tortoise", "Hea1951Ter", {
        onSuccess: () => {
          setIsConnected(true);

          mqtt.client.subscribe("HeaterStatus");
          mqtt.client.subscribe("gaugeHours");
          mqtt.client.subscribe("gaugeMinutes");
        },
        onFailure: (error) => {
          // console.error("Failed to connect to MQTT broker", error);
          setIsConnected(false);
        },
      });

      setMqttService(mqtt);

      return () => {
        // Disconnect MQTT when the screen is unfocused
        if (mqtt) {
          mqtt.disconnect();
        }
        setIsConnected(false); // Reset connection state
      };
    }, [onMessageArrived])
  );

  function handleReconnect() {
    if (mqttService) {
      mqttService.reconnect();
      mqttService.reconnectAttempts = 0;
    } else {
      // console.log("Heater ON/OFF line 110 MQTT Service is not initialized");
    }
  }

  useEffect(() => {
    const loadData = async () => {
      try {
        const savedData = await AsyncStorage.getItem("chartData");
        if (savedData) {
          setData(JSON.parse(savedData));
        }
      } catch (error) {
        // console.error("Failed to load data", error);
      }
    };

    loadData();
  }, []);

  useEffect(() => {
    const saveData = async () => {
      try {
        await AsyncStorage.setItem("chartData", JSON.stringify(data));
      } catch (error) {
        // console.error("Failed to save data", error);
      }
    };

    saveData();
  }, [data]);

  return (
    <SafeAreaView style={styles.graphContainer}>
      <View>
        <Text style={styles.header}> HeaterStatus </Text>

        <Text style={styles.timeText}>Hours: Minutes</Text>
        <Text style={styles.time}>
          {gaugeHours}:{gaugeMinutes.toString().padStart(2, "0")}
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
      <CustomLineChart data={data} textcolor1={textColor} curved={false} />
      <TouchableOpacity
        style={styles.reconnectButton}
        onPress={handleReconnect}
      >
        <Text style={styles.reconnectText}>Reconnect</Text>
      </TouchableOpacity>
      <StatusBar style="auto" />
    </SafeAreaView>
  );
};
export default Heater_ON_OFF_Graph;
