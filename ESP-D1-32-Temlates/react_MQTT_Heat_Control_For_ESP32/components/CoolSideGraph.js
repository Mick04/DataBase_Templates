import { StatusBar } from "expo-status-bar";
import React, { useState, useEffect, useCallback } from "react";
import {
  SafeAreaView,
  TouchableOpacity,
  Text,
  View,
  Alert,
} from "react-native";
import { useFocusEffect } from "@react-navigation/native";
import AsyncStorage from "@react-native-async-storage/async-storage";
import MqttService from "./MqttService";
import CustomLineChart from "./CustomLineChart"; // Import the reusable component
import { styles } from "../Styles/styles";

const CoolSideGraph = () => {
  const [mqttService, setMqttService] = useState(null);
  const [ESP32coolSide, setCoolSideTemp] = useState("");
  const [ESP32gaugeHours, setGaugeHours] = useState(0);
  const [ESP32gaugeMinutes, setGaugeMinutes] = useState(0);
  const [isConnected, setIsConnected] = useState(false);
  const [data, setData] = useState([
    { value: -10, label: "10:00", dataPointText: "-10 c˚" },
  ]);

  // Define the onMessageArrived callback
  const onMessageArrived = useCallback(
    (message) => {
      if (message.destinationName === "ESP32coolSide") {
        const newTemp = parseFloat(message.payloadString).toFixed(1);
        const lastTemp = data.length > 0 ? data[data.length - 1].value : null;

        if (lastTemp === null || Math.abs(newTemp - lastTemp) >= 0.01) {
          const formattedTemp = parseFloat(newTemp); // Convert back to number
          setCoolSideTemp(formattedTemp);
          setData((prevData) => [
            ...prevData,
            {
              value: formattedTemp,
              label: new Date().toLocaleTimeString([], {
                hour: "2-digit",
                minute: "2-digit",
              }),
              dataPointText: `${formattedTemp} c˚`,
            },
          ]);
        }
      }
      if (message.destinationName === "ESP32gaugeHours") {
        setGaugeHours(parseInt(message.payloadString));
      }
      if (message.destinationName === "ESP32gaugeMinutes") {
        setGaugeMinutes(parseInt(message.payloadString));
      }
    },
    [data, ESP32coolSide]
  );

  useFocusEffect(
    useCallback(() => {
      console.log("CoolSideScreen is focused");

      // Initialize the MQTT service
      const mqtt = new MqttService(onMessageArrived, setIsConnected);
      mqtt.connect("ESP32Tortiose", "Hea1951TerESP32", {
        onSuccess: () => {
          setIsConnected(true);
          mqtt.client.subscribe("ESP32coolSide");
          mqtt.client.subscribe("ESP32gaugeHours");
          mqtt.client.subscribe("ESP32gaugeMinutes");
        },
        onFailure: (error) => {
          // console.error("Failed to connect to MQTT broker", error);
          setIsConnected(false);
        },
      });

      setMqttService(mqtt);

      return () => {
        console.log("CoolSideScreen is unfocused");
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
      // console.error("MQTT service is not initialized");
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
      <Text style={styles.ESPHeader}>MQTT_Heat_Control ESP32</Text>
        <Text style={styles.header}>CoolSide Temperature</Text>
        <Text style={styles.timeText}>Hours: Minutes</Text>
        <Text style={styles.time}>
          {ESP32gaugeHours}:{ESP32gaugeMinutes.toString().padStart(2, "0")}
        </Text>
      </View>
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
      <CustomLineChart data={data} GraphTextcolor={'green'} curved={true} />
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
export default CoolSideGraph;