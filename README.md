# Inventory Management System

A robust, console-based Inventory Management System written in C. This system allows administrators to manage products, track inventory, record sales and purchases, and generate reports. Data is persistently stored in text files.

## Features

- **Product Management:** Add, view, search, update, and delete products.
- **Transaction Logging:** Record purchases (restocking) and sales with automatic timestamps.
- **Inventory Tracking:** Check low-stock alerts and calculate total inventory value.
- **Reporting:** Generate daily sales reports and view the full transaction history.
- **Data Persistence:** Data is automatically loaded on startup and can be saved manually or upon exiting.
- **Admin Authentication:** Simple login mechanism to restrict access.

## How to Use

1. **Compile the program:**
   Compile `main.c` using a C compiler like GCC.
   ```bash
   gcc -Wall -Wextra -o InventorySystem main.c
   ```
2. **Run the program:**
   ```bash
   ./InventorySystem
   ```
3. **Login:**
   Use the default administrator credentials:
   - **Username:** `admin`
   - **Password:** `admin123`
4. **Main Menu:**
   Choose an option from 1 to 12 by entering the corresponding number.
5. **Save & Exit:**
   Always use option 12 to safely save your data and exit the program.

## Data Files

The system uses two plain text files to store data persistently:
- `inventory.txt`: Stores product information (ID, Name, Price, Quantity).
- `sales_log.txt`: Stores transaction logs (Transaction ID, Date/Time, Product ID, Type, Quantity, Total Amount).

## System Functions Breakdown

Here is a detailed explanation of every function in the codebase:

### Core Functions
- `main()`: The entry point of the program. It loads data, triggers the admin login, and opens the main menu.
- `loadData()`: Reads `inventory.txt` and `sales_log.txt` into memory arrays (`inventory` and `sales_log`) at startup.
- `saveData()`: Writes the current in-memory data back to `inventory.txt` and `sales_log.txt`.
- `adminLogin()`: Prompts the user for a username and password. Loops until the correct credentials (`admin` / `admin123`) are provided.
- `mainMenu()`: Displays the interactive 12-option menu and handles user input via a `switch` statement.

### Product Management Functions
- `addProduct()`: Prompts the user for new product details (ID, Name, Price, Quantity) and adds it to the inventory. Checks for duplicate IDs and storage limits.
- `viewProducts()`: Displays a formatted, tabular list of all products currently in the inventory.
- `searchProduct()`: Asks for a Product ID and displays the corresponding product details if found.
- `updateProduct()`: Allows the user to modify the price and quantity of an existing product.
- `deleteProduct()`: Removes a product from the inventory based on its ID by shifting the array elements.
- `findProductIndex(int id)`: A utility function that returns the array index of a product given its ID, or `-1` if not found.

### Transaction Functions
- `recordPurchase()`: Records a stock addition. Prompts for Product ID and quantity, updates the inventory, and logs a 'P' (Purchase) transaction with the current timestamp.
- `recordSale()`: Records a stock reduction. Prompts for Product ID and quantity, checks if sufficient stock exists, decrements the inventory, and logs an 'S' (Sale) transaction.
- `viewSalesHistory()`: Displays a complete, tabular list of all recorded transactions (both purchases and sales).

### Reporting & Analytics Functions
- `checkLowStockAlerts()`: Scans the inventory and warns the user about any products whose quantity is at or below the `LOW_STOCK_THRESHOLD` (10).
- `calculateInventoryValue()`: Computes and prints the total monetary value of all current stock (sum of `price * quantity` for all products).
- `generateDailyReport()`: Calculates and displays the total number of items sold and the total revenue generated for the current day.

### Utility Functions
- `getCurrentTime(char* buffer)`: Fetches the current system time and formats it as `YYYY-MM-DD_HH:MM` for transaction logging.
- `getCurrentDate(char* buffer)`: Fetches the current system date and formats it as `YYYY-MM-DD` for daily reporting.
