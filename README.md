# Producer_Consumer Problem

#### *Name: Lara HossamElDin Mostafa Abdou Hassan	 ID: 6853*

#### <u>Problem Statement</u>

Each student is required to implement the **Bounded-Buffer Producer/Consumer problem**. You may have up to 20 producers. Each Producer would declare the live price of a commodity (e.g., GOLD, SILVER, CRUDEOIL, COTTON, …). **One** Consumer would show a dashboard for the prices of all
commodities. Producers and Consumer would run indefinitely sharing the prices through **shared memory**.

<u>**Producers**</u>

Each producer is supposed to continuously declare the price of one commodity. For simplicity, we assume
that each commodity price follows a normal distribution with parameters (𝜇, 𝜎 2 ). Therefore, producers
will generate a new random price, share it with the consumer, and then sleep for an interval before
producing the next price. All producers are processes running the same codebase. Producers are to be run concurrently, either in separate terminals, or in the background. While running a producer, you will specify the following command line arguments:
• Commodity name (e.g., GOLD – Assume no more than 10 characters.)
• Commodity Price Mean; 𝜇 – a double value.
• Commodity Price Standard Deviation; 𝜎 – a double value.
• Length of the sleep interval in milliseconds; T – an integer.
Therefore, the command “./producer NATURALGAS 7.1 0.5 200” would run a producer that declares the
current price of Natural Gas every 200ms according to a normal distribution with parameters (mean=0.5
and variance=0.25)

<u>**Consumer**</u>
The consumer is to print the current price of each commodity, along the average of the current and past
4 readings. An Up/Down arrow to show whether the current Price (AvgPrice) got increased or decreased
from the prior one. Until you receive current prices, use 0.00 for the current price of any commodity.
For simplicity, let’s limit the commodities to GOLD, SILVER, CRUDEOIL, NATURALGAS, ALUMINIUM,
COPPER, NICKEL, LEAD, ZINC, MENTHAOIL, and COTTON. Show the commodities in alphabetical order.

### Files in the repo

* main.cc - creates the shared memory
* producer.cc - creates a new producer
* consumer.cc - shows the dashboard of commodities

### How to run the program?

