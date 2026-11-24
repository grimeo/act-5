const express = require("express");
const router = express.Router();

// Sample device data stored in memory
let devices = [{ deviceId: "ESP32-01", led: "off" }];

// GET - GET /api/devices/:id
router.get("/:id", (req, res) => {
  const deviceId = req.params.id;

  const device = devices.find((item) => item.deviceId === deviceId);

  if (!device) {
    return res.status(404).json({
      error: "Device not found",
    });
  }

  res.json({ data: device });
});

// UPDATE - PUT /api/devices/:id
router.put("/:id", (req, res) => {
  const deviceId = req.params.id;
  const { led } = req.body || {};

  let device = devices.find((item) => item.deviceId === deviceId);

  if (!device) {
    return res.status(404).json({
      error: "Device not found",
    });
  }

  if (led !== "on" && led !== "off") {
    return res.status(400).json({
      error: 'led must be either "on" or "off"',
    });
  }

  device.led = led;

  res.json({
    message: "Device updated",
    data: device,
  });
});

module.exports = router;
