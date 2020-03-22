

unsigned long n;      //numero inserito oltre il quale non andare
unsigned long timeOne;
unsigned long count;


void setup() {
  Serial.begin(57600);
}

void loop() {
  Serial.print("Up to what number do you want to find prime numbers? ");

  while (Serial.available() <= 0);

  n = Serial.parseInt();
  Serial.println(n);
  Serial.println("Started crunching numbers (trial division)...");
  timeOne = millis();
  count = 0;

  for (unsigned long i = 0; i <= n; i++) {
    if (IsPrimeFor(i)) {
      Serial.print(i);
      Serial.print("\t");
      count++;
      if (count % 10 == 0) Serial.println();
    }
  }

  timeOne = millis() - timeOne;
  Serial.println();
  Serial.print("It took me: ");
  Serial.print(timeOne);
  Serial.print(" millis. Found: ");
  Serial.print(count);
  Serial.println(" prime numbers");
}


boolean IsPrimeFor(unsigned long n) {
  if (n % 2 == 0) {
    if (n == 2)
      return true; //Two is the only even prime number
    return false; //Even number
  }

  unsigned long limit = (unsigned long)floor(sqrt((float)n));
  if (n == limit * limit)
    return false; //Perfect square

  unsigned long p = 3;
  for (; p <= limit; p += 2)
    if (n % p == 0)
      return false;

  return true;
}



boolean IsPrimeWhile(unsigned long n) {
  if (n % 2 == 0) {
    if (n == 2)
      return true; //Two is the only even prime number
    return false; //Even number
  }

  unsigned long limit = (unsigned long)floor(sqrt((float)n));
  if (n == limit * limit)
    return false; //Perfect square

  unsigned long p = 3;
  while (p <= limit && n % p != 0)
    p += 2;

  if (p > limit)
    return true;
  else
    return false;
}

