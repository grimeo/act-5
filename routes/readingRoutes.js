const express = require("express");
const router = express.Router();
// Sample data stored in memory (it resets every time the server restarts)

let readings = [
  { id: 1, deviceId: "ESP32-01", temperature: 28.5, humidity: 65 },
  { id: 2, deviceId: "ESP32-01", temperature: 29.1, humidity: 63 },
];
let nextId = 3;
// Helper: checks the JSON body sent by Postman or the ESP32
function validateReading(body) {
  const { deviceId, temperature, humidity } = body || {};
  if (typeof deviceId !== "string" || deviceId.trim() === "") {
    return "deviceId must be a non-empty text value";
  }
  if (typeof temperature !== "number") {
    return "temperature must be a number";
  }
  if (typeof humidity !== "number") {
    return "humidity must be a number";
  }
  return null; // no error
}
// CREATE - POST /api/readings
router.post("/", (req, res) => {
  const error = validateReading(req.body);
  if (error) {
    return res.status(400).json({ error: error });
  }
  const { deviceId, temperature, humidity } = req.body;
  const reading = { id: nextId++, deviceId, temperature, humidity };
  readings.push(reading);
  res.status(201).json({ message: "Reading created", data: reading });
});
// READ ALL - GET /api/readings
router.get("/", (req, res) => {
  res.json({ count: readings.length, data: readings });
});
4;
// READ ONE - GET /api/readings/:id
router.get("/:id", (req, res) => {
  const id = Number(req.params.id);
  const reading = readings.find((item) => item.id === id);
  if (!reading) {
    return res.status(404).json({ error: "Reading not found" });
  }
  res.json({ data: reading });
});
// UPDATE - PUT /api/readings/:id
router.put("/:id", (req, res) => {
  const id = Number(req.params.id);
  const reading = readings.find((item) => item.id === id);
  if (!reading) {
    return res.status(404).json({ error: "Reading not found" });
  }
  const error = validateReading(req.body);
  if (error) {
    return res.status(400).json({ error: error });
  }
  reading.deviceId = req.body.deviceId;
  reading.temperature = req.body.temperature;
  reading.humidity = req.body.humidity;
  res.json({ message: "Reading updated", data: reading });
});
// DELETE - DELETE /api/readings/:id
router.delete("/:id", (req, res) => {
  const id = Number(req.params.id);
  const index = readings.findIndex((item) => item.id === id);
  if (index === -1) {
    return res.status(404).json({ error: "Reading not found" });
  }
  const removed = readings.splice(index, 1)[0];
  res.json({ message: "Reading deleted", data: removed });
});
module.exports = router;
