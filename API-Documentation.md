# Weather App Client-Server Functionality Specification

## Overview

This specification outlines the primary functionalities and interactions for a weather app with a client-server architecture. The system consists of two components:

- **Client**: A user-facing application (mobile, web, or desktop) to display weather information and capture user input.
- **Server**: A backend that fetches weather data from third-party sources/APIs, processes requests, and sends responses to clients.

---

## Functional Requirements

### 1. User Interaction

#### 1.1 Location Input
- User can:
    - Enter a city name, location, or allow automatic geolocation.
    - View a list of recently searched locations (persistent or session-based).

#### 1.2 Viewing Weather Information (TBD)
- User receives:
    - Current temperature.
    - Weather condition description (e.g., sunny, cloudy, rainy).
    - Wind speed and direction.
    - Humidity.
    - "Feels like" temperature.
    - Local time at location.
    - Sunrise and sunset times.

#### 1.3 Error Handling (TBD)
- User receives meaningful error messages if:
    - Location is not found.
    - Network issues occur.
    - API/server returns an error.

### 2. Client-Server Interaction

#### 2.1 API Endpoints
- **Location** could be some internal id or just stick with lat lon
##### 2.1.1 Get Cities
- `GET v1/cities`
    - **Query params**: `search=<string>`
    - **Response**: List of matching cities and their cords and a internal id.

##### 2.1.2 New City
- `POST v1/cities`
    - **Request Body**: `{name="<string>", lat=<float>, lon=<float>}`
    - **Response**: HTTP Code

##### 2.1.3 Get Weather
- `GET v1/cities/weather`
    - **Query params**: `location=<string>` or `lat=<float>&lon=<float>`
    - **Response**: Weather data

#### 2.2 Data Format

- All API responses are JSON.
- Fields include: temperature, condition, humidity, wind, precipitation, sunrise, sunset, icon URL, etc.

### 3. Server Responsibilities

- Validate and sanitize all incoming requests.
- Query weather data from external providers.
- Cache responses as appropriate to reduce load.
- Handle API rate limiting.
- Standardize and format data before responding to the client.
- Log and monitor for errors.

## Example Interaction

```
Client Request:
GET /cities/<city>/weather?lat=<float>&lon=<float> -- NOT YET IMPLEMENTED
OR
GET /weather?lat=<float>&lon=<float> -- IMPLEMENTED

Example API usage:

GET /weather?latitude=57.1579&longitude=39.3355 HTTP/1.1

Maximum latitude and longitude: -90 - +90, going above it will throw a 400 Bad Request

Server Response:
{
  "location": "Rostovsky District, Russia", -- Will be Unknown if location cant be found
  "temperature": 0.6,
  "unit": "C", -- Will always be C right now
  "condition": "Cloudy", -- Clear Sky, Cloudy, Fog, Rain, Snow, Showers, Thunderstorms.
  "feels_like": -2,
  "humidity": 92,
  "wind": {
    "speed": 6, -- km/h
    "direction": "227" -- Uses degrees instead of the common 'NE'/'N'/'S', etc.
  },
  "sunrise": "08:45",
  "sunset": "15:44",
  "icon_url": "https://weatherapp.com/icons/cloudy.png" -- Not yet implemented, do not use this.
}
