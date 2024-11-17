const express = require('express');
const cors = require('cors');
const app = express();
const port = 3000;

app.use(cors()); // Enable CORS for all origins

app.use(express.json());

let locationData = { latitude: null, longitude: null };

app.post('/location', (req, res) => {
  locationData = req.body;
  console.log(locationData);
  res.send('Location data received!');
});

app.get('/location', (req, res) => {
  res.json(locationData);
});

app.listen(port, () => {
  console.log(`Server running on http://localhost:${port}`);
});