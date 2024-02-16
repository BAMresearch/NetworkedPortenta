// Utilities.ino

void calcDewpoint() {
  float b = 17.62; // sonntag set:
  float c = 243.12;
  float gamma = (b * T / (c + T)) + log(H / 100.0);
  dp = (c * gamma) / (b - gamma);
}
