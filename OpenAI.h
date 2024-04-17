#ifndef OPENAI_H
#define OPENAI_H

#include "Utils.h"

const String openAI_URL = "https://api.openai.com/v1/";
String openAIKey = "";

String readOpenAIKey(String apiKeyFile) {
  openAIKey = readFile(apiKeyFile);
  return openAIKey;
}

void openAI_TTS(String text, String filePath) {
  if (openAIKey == "") {
    Serial.println("OpenAI key not set, please call readOpenAIKey() first");
    return;
  }

  String url = openAI_URL + "audio/speech";

  // Prepare the JSON payload
  DynamicJsonDocument doc(1024);
  doc["model"] = "tts-1";
  doc["input"] = text;
  doc["voice"] = "nova";
  doc["response_format"] = "wav";
  String payload;
  serializeJson(doc, payload);
  String contentType = "application/json";

  // Configure the secure client and make the request
  WiFiClientSecure client;
  client.setInsecure();  // Disable certificate verification (not recommended
                         // for production)
  HTTPClient http;
  http.begin(client, url);  // Start the client with URL

  // Set headers
  http.addHeader("Content-Type", contentType);
  http.addHeader("Authorization", "Bearer " + openAIKey);

  int httpCode = http.POST(payload);  // Perform the POST request

  if (httpCode == HTTP_CODE_OK) {
    // Open the file for writing in binary mode
    Serial.println("OpenAI TTS request successful, writing to file...");
    File file = LittleFS.open(filePath, "w+");
    if (!file) {
      Serial.println("Failed to open file for writing");
      http.end();  // End the connection
      return;
    }

    // Stream the response into the file
    // This handles the binary data appropriately by treating it as a stream of
    // bytes
    http.writeToStream(&file);
    file.close();  // Make sure to close the file to save the data
    Serial.println("File written successfully");
  } else {
    Serial.print("HTTP POST failed, error: ");
    Serial.println(http.errorToString(httpCode).c_str());
  }

  http.end();  // End the connection
}

String openAIChat(String text) {
  if (openAIKey == "") {
    Serial.println("OpenAI key not set, please call readOpenAIKey() first");
    return "";
  }

  String url = openAI_URL + "chat/completions";

  // Prepare the JSON payload
  DynamicJsonDocument doc(1024);
  doc["model"] = "gpt-3.5-turbo";
  doc["messages"] = JsonArray();
  doc["messages"].add(JsonObject());
  doc["messages"][0]["role"] = "system";
  doc["messages"][0]["content"] =
      "You are an AI ham radio operator."
      "Your call sign is wsce496."
      "Use the call sign to refer to yourself."
      "Use the NATO phonetic alphabet when reciting letters."
      "You are here to help the user with their questions."
      "You can also tell jokes."
      "Be as funny and punny as possible when telling jokes"
      "try to keep jokes radio related.";
  doc["messages"].add(JsonObject());
  doc["messages"][1]["role"] = "user";
  doc["messages"][1]["content"] = text;
  doc["max_tokens"] = 60;
  String payload;
  serializeJson(doc, payload);
  String contentType = "application/json";

  // Make the request
  String response =
      makeHTTPSRequest("POST", url, openAIKey, contentType, payload);

  DynamicJsonDocument resDoc(1024);
  deserializeJson(resDoc, response);
  if (resDoc["choices"][0]["message"]["content"].is<String>()) {
    return resDoc["choices"][0]["message"]["content"].as<String>();
  } else {
    return "Error parsing response";
  }

  return "Sorry, I didn't understand that. Please try again.";
}

#endif OPENAI_H