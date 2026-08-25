#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


/* Cross-Platform Console Management */
#ifdef _WIN32
#include <conio.h>
#define CLEAR_SCREEN() system("cls")
#else
#include <termios.h>
#include <unistd.h>
#define CLEAR_SCREEN() system("clear")

/**
 * getch - Reads a single character from terminal without echo on UNIX systems.
 * Elements:
 * - struct termios oldattr, newattr: Holds original and modified terminal I/O
 * settings.
 * - tcgetattr/tcsetattr: Reads/applies terminal state.
 * - ICANON & ECHO: Flags cleared to disable line buffering and key echoing.
 */
int getch(void) {
  struct termios oldattr, newattr;
  int ch;
  tcgetattr(STDIN_FILENO, &oldattr);
  newattr = oldattr;
  newattr.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newattr);
  ch = getchar();
  tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
  return ch;
}
#endif

#define MAX_NAME_LEN 64
#define LOW_STOCK_THRESHOLD 10
#define MAX_PRODUCTS 1000
#define MAX_TRANSACTIONS 5000

/* Data Structures */
typedef struct {
  int id;                  // Unique identifier for the product
  char name[MAX_NAME_LEN]; // Product label (supports spaces)
  double price;            // Unit retail price
  int quantity;            // Available physical inventory units
} Product;

typedef struct {
  int transaction_id;  // Auto-incremented transaction serial ID
  char date_time[20];  // Timestamp formatted as "YYYY-MM-DD HH:MM:SS"
  int product_id;      // Associated Product ID
  char type;           // 'S' for Sale, 'P' for Purchase/Restock
  int quantity;        // Quantity transacted
  double unit_price;   // Effective price per unit for this transaction
  double total_amount; // Computed total value (quantity * unit_price)
} Transaction;

/* In-Memory Database Storage */
Product inventory[MAX_PRODUCTS];
int product_count = 0;

Transaction sales_log[MAX_TRANSACTIONS];
int transaction_count = 0;

/* Function Declarations */
void loadData(void);
void saveData(void);
void adminLogin(void);
void mainMenu(void);

void addProduct(void);
void viewProducts(void);
void searchProduct(void);
void updateProduct(void);
void deleteProduct(void);

void recordPurchase(void);
void recordSale(void);

void checkLowStockAlerts(void);
void viewSalesHistory(void);
void calculateInventoryValue(void);
void generateDailyReport(void);

void clearInputBuffer(void);
void getStringInput(const char *prompt, char *output, size_t max_len);
int getIntInput(const char *prompt, int min_val, int max_val);
double getDoubleInput(const char *prompt, double min_val, double max_val);
void pausePrompt(void);
int findProductIndex(int id);
void getCurrentTime(char *buffer, size_t size);
void getCurrentDate(char *buffer, size_t size);
void getMaskedPassword(char *buffer, size_t max_len);

/**
 * main - Program entry point.
 * Coordinates initialization, authentication, primary execution loop, and safe
 * termination.
 */
int main(void) {
  loadData();   // Read persisted files into memory
  adminLogin(); // Authenticate user credentials
  mainMenu();   // Main interactive lifecycle
  saveData();   // Ensure state synchronization on termination
  return 0;
}

// -------------------------------------------------------------
// Helper & Input Sanitization Functions
// -------------------------------------------------------------

/**
 * clearInputBuffer - Cleans lingering newline or extra characters from stdin.
 */
void clearInputBuffer(void) {
  int c;
  while ((c = getchar()) != '\n' && c != EOF)
    ;
}

/**
 * pausePrompt - Halts execution until user presses Enter, preventing menu
 * redraw skips.
 */
void pausePrompt(void) {
  printf("\nPress [Enter] to continue...");
  clearInputBuffer();
}

/**
 * getStringInput - Robust multi-word string ingestion avoiding buffer
 * overflows. Elements:
 * - prompt: Directive text presented to user.
 * - output: Destination char array.
 * - max_len: Maximum capacity of the destination buffer.
 */
void getStringInput(const char *prompt, char *output, size_t max_len) {
  while (1) {
    printf("%s", prompt);
    if (fgets(output, (int)max_len, stdin) != NULL) {
      size_t len = strlen(output);
      if (len > 0 && output[len - 1] == '\n') {
        output[len - 1] = '\0'; // Remove trailing newline
        len--;
      } else {
        clearInputBuffer(); // Clear stream overflow
      }
      if (len > 0)
        return; // Reject blank inputs
    }
    printf("Input cannot be empty. Please try again.\n");
  }
}

/**
 * getIntInput - Validates integer input within bounds.
 * Elements:
 * - buffer: Temporary string holding user input line.
 * - extra: Trailing character check to reject dirty formats (e.g., "12abc").
 * - min_val / max_val: Allowed range constraints.
 */
int getIntInput(const char *prompt, int min_val, int max_val) {
  int value;
  char buffer[128];
  while (1) {
    printf("%s", prompt);
    if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
      char extra;
      if (sscanf(buffer, "%d %c", &value, &extra) == 1) {
        if (value >= min_val && value <= max_val) {
          return value;
        }
        printf("Error: Value must be between %d and %d.\n", min_val, max_val);
      } else {
        printf("Error: Invalid numeric input.\n");
      }
    }
  }
}

/**
 * getDoubleInput - Validates floating-point double input within bounds.
 */
double getDoubleInput(const char *prompt, double min_val, double max_val) {
  double value;
  char buffer[128];
  while (1) {
    printf("%s", prompt);
    if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
      char extra;
      if (sscanf(buffer, "%lf %c", &value, &extra) == 1) {
        if (value >= min_val && value <= max_val) {
          return value;
        }
        printf("Error: Value must be between %.2f and %.2f.\n", min_val,
               max_val);
      } else {
        printf("Error: Invalid decimal input.\n");
      }
    }
  }
}

/**
 * getMaskedPassword - Hides password inputs by printing '*' to the console.
 * Elements:
 * - idx: Current length of password entered.
 * - \b \b: Backspace-space-backspace escape sequence to erase asterisks
 * visually.
 */
void getMaskedPassword(char *buffer, size_t max_len) {
  size_t idx = 0;
  int ch;
  while (1) {
    ch = getch();
    if (ch == '\r' || ch == '\n') { // Enter key pressed
      buffer[idx] = '\0';
      printf("\n");
      break;
    } else if (ch == '\b' || ch == 127) { // Handle backspace
      if (idx > 0) {
        idx--;
        printf("\b \b");
      }
    } else if (idx < max_len - 1 && isprint(ch)) {
      buffer[idx++] = (char)ch;
      printf("*");
    }
  }
}

/**
 * getCurrentTime - Populates buffer with current date and time.
 */
void getCurrentTime(char *buffer, size_t size) {
  time_t rawtime;
  struct tm *timeinfo;
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  strftime(buffer, size, "%Y-%m-%d %H:%M:%S", timeinfo);
}

/**
 * getCurrentDate - Populates buffer with date only (YYYY-MM-DD).
 */
void getCurrentDate(char *buffer, size_t size) {
  time_t rawtime;
  struct tm *timeinfo;
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  strftime(buffer, size, "%Y-%m-%d", timeinfo);
}

/**
 * findProductIndex - Searches product array for matching ID.
 * Returns: Array index if found, -1 otherwise.
 */
int findProductIndex(int id) {
  for (int i = 0; i < product_count; i++) {
    if (inventory[i].id == id) {
      return i;
    }
  }
  return -1;
}

// -------------------------------------------------------------
// File Persistence (TXT Delimited Format)
// -------------------------------------------------------------

/**
 * loadData - Reads delimited records from text files into memory.
 */
void loadData(void) {
  product_count = 0;
  FILE *f1 = fopen("inventory.txt", "r");
  if (f1 != NULL) {
    char line[256];
    while (fgets(line, sizeof(line), f1) && product_count < MAX_PRODUCTS) {
      line[strcspn(line, "\r\n")] = '\0';
      if (strlen(line) == 0)
        continue;

      Product p;
      if (sscanf(line, "%d|%63[^|]|%lf|%d", &p.id, p.name, &p.price,
                 &p.quantity) == 4) {
        inventory[product_count++] = p;
      }
    }
    fclose(f1);
  }

  transaction_count = 0;
  FILE *f2 = fopen("sales_log.txt", "r");
  if (f2 != NULL) {
    char line[256];
    while (fgets(line, sizeof(line), f2) &&
           transaction_count < MAX_TRANSACTIONS) {
      line[strcspn(line, "\r\n")] = '\0';
      if (strlen(line) == 0)
        continue;

      Transaction t;
      if (sscanf(line, "%d|%19[^|]|%d|%c|%d|%lf|%lf", &t.transaction_id,
                 t.date_time, &t.product_id, &t.type, &t.quantity,
                 &t.unit_price, &t.total_amount) == 7) {
        sales_log[transaction_count++] = t;
      }
    }
    fclose(f2);
  }
}

/**
 * saveData - Writes memory data out to pipe-delimited text files.
 */
void saveData(void) {
  FILE *f1 = fopen("inventory.txt", "w");
  if (f1 != NULL) {
    for (int i = 0; i < product_count; i++) {
      fprintf(f1, "%d|%s|%.2f|%d\n", inventory[i].id, inventory[i].name,
              inventory[i].price, inventory[i].quantity);
    }
    fclose(f1);
  }

  FILE *f2 = fopen("sales_log.txt", "w");
  if (f2 != NULL) {
    for (int i = 0; i < transaction_count; i++) {
      fprintf(f2, "%d|%s|%d|%c|%d|%.2f|%.2f\n", sales_log[i].transaction_id,
              sales_log[i].date_time, sales_log[i].product_id,
              sales_log[i].type, sales_log[i].quantity, sales_log[i].unit_price,
              sales_log[i].total_amount);
    }
    fclose(f2);
  }
}

// -------------------------------------------------------------
// Authentication & Menu System
// -------------------------------------------------------------

/**
 * adminLogin - Locks system behind username/password check.
 */
void adminLogin(void) {
  char username[32];
  char password[32];
  int attempts = 0;

  while (attempts < 3) {
    CLEAR_SCREEN();
    printf("====================================================\n");
    printf("      INVENTORY MANAGEMENT SYSTEM - AUTHENTICATION   \n");
    printf("====================================================\n");

    getStringInput("Admin Username: ", username, sizeof(username));
    printf("Password: ");
    getMaskedPassword(password, sizeof(password));

    if (strcmp(username, "admin") == 0 && strcmp(password, "admin123") == 0) {
      printf("\n[+] Access granted. Welcome, Admin.\n");
      pausePrompt();
      return;
    } else {
      attempts++;
      printf("\n[-] Invalid credentials. (%d/3 attempts left)\n", 3 - attempts);
      pausePrompt();
    }
  }

  printf("\n[!] Maximum login attempts reached. Exiting program...\n");
  exit(EXIT_FAILURE);
}

/**
 * mainMenu - Displays primary terminal interface and routes execution.
 */
void mainMenu(void) {
  int choice;
  do {
    CLEAR_SCREEN();
    printf("====================================================\n");
    printf("        INVENTORY & SALES MANAGEMENT SYSTEM         \n");
    printf("====================================================\n");
    printf(" 1.  Add New Product\n");
    printf(" 2.  View All Products (Inventory Table)\n");
    printf(" 3.  Search Product (by ID or Name)\n");
    printf(" 4.  Update Product Details\n");
    printf(" 5.  Delete Product\n");
    printf(" --------------------------------------------------\n");
    printf(" 6.  Record Stock Purchase (Restock)\n");
    printf(" 7.  Record Sale (Bill/Invoice)\n");
    printf(" --------------------------------------------------\n");
    printf(" 8.  Low-Stock Alerts\n");
    printf(" 9.  View Full Audit / Transaction History\n");
    printf(" 10. Calculate Total Inventory Valuation\n");
    printf(" 11. Generate Daily Analytics Report\n");
    printf(" 12. Save & Exit\n");
    printf("====================================================\n");

    choice = getIntInput("Select an option (1-12): ", 1, 12);

    CLEAR_SCREEN();
    switch (choice) {
    case 1:
      addProduct();
      break;
    case 2:
      viewProducts();
      break;
    case 3:
      searchProduct();
      break;
    case 4:
      updateProduct();
      break;
    case 5:
      deleteProduct();
      break;
    case 6:
      recordPurchase();
      break;
    case 7:
      recordSale();
      break;
    case 8:
      checkLowStockAlerts();
      break;
    case 9:
      viewSalesHistory();
      break;
    case 10:
      calculateInventoryValue();
      break;
    case 11:
      generateDailyReport();
      break;
    case 12:
      saveData();
      printf("\nAll records successfully synced to inventory.txt and "
             "sales_log.txt.\n");
      printf("Thank you for using the system. Goodbye!\n");
      break;
    }
    if (choice != 12) {
      pausePrompt();
    }
  } while (choice != 12);
}

// -------------------------------------------------------------
// Core Functional Modules
// -------------------------------------------------------------

/**
 * addProduct - Registers a new unique product into the inventory.
 */
void addProduct(void) {
  printf("====================================================\n");
  printf("                  ADD NEW PRODUCT                   \n");
  printf("====================================================\n");

  if (product_count >= MAX_PRODUCTS) {
    printf("[!] Inventory database is full.\n");
    return;
  }

  int id = getIntInput("Enter Product ID (Positive Integer): ", 1, 999999);
  if (findProductIndex(id) != -1) {
    printf("[-] Error: Product ID %d is already registered.\n", id);
    return;
  }

  Product p;
  p.id = id;
  getStringInput("Enter Product Name: ", p.name, sizeof(p.name));
  p.price = getDoubleInput("Enter Unit Price ($): ", 0.01, 1000000.0);
  p.quantity = getIntInput("Enter Initial Stock Quantity: ", 0, 100000);

  inventory[product_count++] = p;
  saveData();
  printf("\n[+] Product '%s' (ID: %d) successfully registered!\n", p.name,
         p.id);
}

/**
 * viewProducts - Renders a formatted tabular overview of all inventory stock.
 */
void viewProducts(void) {
  printf("====================================================================="
         "=====\n");
  printf("                             CURRENT INVENTORY                       "
         "     \n");
  printf("====================================================================="
         "=====\n");
  printf("%-8s | %-28s | %-12s | %-10s | %-10s\n", "ID", "Product Name",
         "Price ($)", "Stock", "Status");
  printf("---------------------------------------------------------------------"
         "-----\n");

  if (product_count == 0) {
    printf(" No items available in inventory.\n");
  } else {
    for (int i = 0; i < product_count; i++) {
      const char *status =
          (inventory[i].quantity <= LOW_STOCK_THRESHOLD) ? "LOW STOCK" : "OK";
      printf("%-8d | %-28.28s | %12.2f | %10d | %-10s\n", inventory[i].id,
             inventory[i].name, inventory[i].price, inventory[i].quantity,
             status);
    }
  }
  printf("====================================================================="
         "=====\n");
  printf("Total registered items: %d\n", product_count);
}

/**
 * searchProduct - Performs lookups via ID or case-insensitive name substrings.
 */
void searchProduct(void) {
  printf("====================================================\n");
  printf("                   SEARCH PRODUCT                   \n");
  printf("====================================================\n");
  printf("1. Search by ID\n");
  printf("2. Search by Name (Substring)\n");
  int mode = getIntInput("Choice (1-2): ", 1, 2);

  if (mode == 1) {
    int id = getIntInput("Enter Product ID: ", 1, 999999);
    int idx = findProductIndex(id);
    if (idx != -1) {
      printf("\n[+] Product Found:\n");
      printf("----------------------------------------\n");
      printf(" ID:          %d\n", inventory[idx].id);
      printf(" Name:        %s\n", inventory[idx].name);
      printf(" Unit Price:  $%.2f\n", inventory[idx].price);
      printf(" Stock:       %d units\n", inventory[idx].quantity);
      printf(" Total Value: $%.2f\n",
             inventory[idx].price * inventory[idx].quantity);
      printf("----------------------------------------\n");
    } else {
      printf("[-] Product ID %d not found.\n", id);
    }
  } else {
    char query[MAX_NAME_LEN];
    getStringInput("Enter search keyword: ", query, sizeof(query));

    printf("\n%-8s | %-28s | %-12s | %-10s\n", "ID", "Product Name",
           "Price ($)", "Stock");
    printf(
        "------------------------------------------------------------------\n");

    int matches = 0;
    for (int i = 0; i < product_count; i++) {
      char temp_name[MAX_NAME_LEN], temp_query[MAX_NAME_LEN];
      strncpy(temp_name, inventory[i].name, MAX_NAME_LEN);
      strncpy(temp_query, query, MAX_NAME_LEN);

      // Convert to lowercase for non-case-sensitive comparison
      for (int j = 0; temp_name[j]; j++)
        temp_name[j] = (char)tolower(temp_name[j]);
      for (int j = 0; temp_query[j]; j++)
        temp_query[j] = (char)tolower(temp_query[j]);

      if (strstr(temp_name, temp_query) != NULL) {
        printf("%-8d | %-28.28s | %12.2f | %10d\n", inventory[i].id,
               inventory[i].name, inventory[i].price, inventory[i].quantity);
        matches++;
      }
    }
    if (matches == 0) {
      printf(" No products matched keyword '%s'.\n", query);
    }
    printf(
        "------------------------------------------------------------------\n");
  }
}

/**
 * updateProduct - Mutates product properties safely.
 */
void updateProduct(void) {
  printf("====================================================\n");
  printf("                 UPDATE PRODUCT                     \n");
  printf("====================================================\n");

  int id = getIntInput("Enter Product ID to update: ", 1, 999999);
  int idx = findProductIndex(id);

  if (idx == -1) {
    printf("[-] Product with ID %d does not exist.\n", id);
    return;
  }

  printf("\nTarget Product: %s (Current Price: $%.2f, Stock: %d)\n",
         inventory[idx].name, inventory[idx].price, inventory[idx].quantity);

  printf("\n1. Update Name\n2. Update Price\n3. Update Stock\n4. Update All\n");
  int opt = getIntInput("Choose what to update (1-4): ", 1, 4);

  if (opt == 1 || opt == 4) {
    getStringInput("Enter New Name: ", inventory[idx].name,
                   sizeof(inventory[idx].name));
  }
  if (opt == 2 || opt == 4) {
    inventory[idx].price =
        getDoubleInput("Enter New Unit Price ($): ", 0.01, 1000000.0);
  }
  if (opt == 3 || opt == 4) {
    inventory[idx].quantity =
        getIntInput("Enter New Stock Quantity: ", 0, 100000);
  }

  saveData();
  printf("\n[+] Product details updated successfully.\n");
}

/**
 * deleteProduct - Removes an item from the database array by shifting elements
 * left.
 */
void deleteProduct(void) {
  printf("====================================================\n");
  printf("                 DELETE PRODUCT                     \n");
  printf("====================================================\n");

  int id = getIntInput("Enter Product ID to delete: ", 1, 999999);
  int idx = findProductIndex(id);

  if (idx == -1) {
    printf("[-] Product with ID %d does not exist.\n", id);
    return;
  }

  printf("[!] WARNING: You are deleting '%s' (ID: %d).\n", inventory[idx].name,
         inventory[idx].id);
  char confirm[10];
  getStringInput("Type 'YES' to confirm deletion: ", confirm, sizeof(confirm));

  if (strcmp(confirm, "YES") == 0) {
    for (int i = idx; i < product_count - 1; i++) {
      inventory[i] = inventory[i + 1];
    }
    product_count--;
    saveData();
    printf("\n[+] Product deleted successfully.\n");
  } else {
    printf("\n[-] Action cancelled.\n");
  }
}

/**
 * recordPurchase - Logs inventory restock transactions and updates quantities.
 */
void recordPurchase(void) {
  printf("====================================================\n");
  printf("            RECORD STOCK PURCHASE (RESTOCK)         \n");
  printf("====================================================\n");

  if (transaction_count >= MAX_TRANSACTIONS) {
    printf("[-] Sales log storage limit reached.\n");
    return;
  }

  int id = getIntInput("Enter Product ID: ", 1, 999999);
  int idx = findProductIndex(id);

  if (idx == -1) {
    printf("[-] Product not found. Register product first.\n");
    return;
  }

  printf("Product Selected: %s (Current Stock: %d)\n", inventory[idx].name,
         inventory[idx].quantity);
  int qty = getIntInput("Enter Quantity to Add: ", 1, 100000);
  double cost =
      getDoubleInput("Enter Purchase Cost Per Unit ($): ", 0.01, 1000000.0);

  inventory[idx].quantity += qty;

  Transaction t;
  t.transaction_id = (transaction_count == 0)
                         ? 1001
                         : sales_log[transaction_count - 1].transaction_id + 1;
  getCurrentTime(t.date_time, sizeof(t.date_time));
  t.product_id = id;
  t.type = 'P';
  t.quantity = qty;
  t.unit_price = cost;
  t.total_amount = qty * cost;

  sales_log[transaction_count++] = t;
  saveData();

  printf("\n[+] Restock logged! New stock level: %d units\n",
         inventory[idx].quantity);
  printf("    Total Cost: $%.2f\n", t.total_amount);
}

/**
 * recordSale - Logs client sale transactions, deducts stock, and displays
 * invoice receipt.
 */
void recordSale(void) {
  printf("====================================================\n");
  printf("                 RECORD POINT OF SALE               \n");
  printf("====================================================\n");

  if (transaction_count >= MAX_TRANSACTIONS) {
    printf("[-] Sales log storage limit reached.\n");
    return;
  }

  int id = getIntInput("Enter Product ID: ", 1, 999999);
  int idx = findProductIndex(id);

  if (idx == -1) {
    printf("[-] Product not found.\n");
    return;
  }

  printf("Product Selected: %s\n", inventory[idx].name);
  printf("Current Price:    $%.2f\n", inventory[idx].price);
  printf("Available Stock:  %d units\n", inventory[idx].quantity);

  if (inventory[idx].quantity <= 0) {
    printf("[-] Product is out of stock.\n");
    return;
  }

  int qty = getIntInput("Enter Quantity to Sell: ", 1, inventory[idx].quantity);

  inventory[idx].quantity -= qty;

  Transaction t;
  t.transaction_id = (transaction_count == 0)
                         ? 1001
                         : sales_log[transaction_count - 1].transaction_id + 1;
  getCurrentTime(t.date_time, sizeof(t.date_time));
  t.product_id = id;
  t.type = 'S';
  t.quantity = qty;
  t.unit_price = inventory[idx].price;
  t.total_amount = qty * inventory[idx].price;

  sales_log[transaction_count++] = t;
  saveData();

  printf("\n====================================================\n");
  printf("                   INVOICE RECEIPT                  \n");
  printf("====================================================\n");
  printf(" Txn ID:      #%d\n", t.transaction_id);
  printf(" Timestamp:   %s\n", t.date_time);
  printf(" Item:        %s (ID: %d)\n", inventory[idx].name, id);
  printf(" Qty Sold:    %d\n", qty);
  printf(" Unit Price:  $%.2f\n", t.unit_price);
  printf(" --------------------------------------------------\n");
  printf(" TOTAL DUE:   $%.2f\n", t.total_amount);
  printf(" Remaining:   %d units in stock\n", inventory[idx].quantity);
  printf("====================================================\n");
}

/**
 * checkLowStockAlerts - Scans products and highlights items at or below
 * threshold.
 */
void checkLowStockAlerts(void) {
  printf("====================================================================="
         "=====\n");
  printf("                           LOW STOCK WARNINGS                        "
         "     \n");
  printf("====================================================================="
         "=====\n");
  printf("%-8s | %-32s | %-12s | %-10s\n", "ID", "Product Name", "Price ($)",
         "Stock");
  printf("---------------------------------------------------------------------"
         "-----\n");

  int alerts = 0;
  for (int i = 0; i < product_count; i++) {
    if (inventory[i].quantity <= LOW_STOCK_THRESHOLD) {
      printf("%-8d | %-32.32s | %12.2f | %10d [ALERT]\n", inventory[i].id,
             inventory[i].name, inventory[i].price, inventory[i].quantity);
      alerts++;
    }
  }

  if (alerts == 0) {
    printf(" All items are adequately stocked (> %d units).\n",
           LOW_STOCK_THRESHOLD);
  }
  printf("====================================================================="
         "=====\n");
}

/**
 * viewSalesHistory - Displays audit history and transaction ledger.
 */
void viewSalesHistory(void) {
  printf("====================================================\n");
  printf("               AUDIT / TRANSACTION LOGS             \n");
  printf("====================================================\n");
  printf("1. View All Records\n");
  printf("2. Filter by Product ID\n");
  int mode = getIntInput("Choice (1-2): ", 1, 2);

  int filter_id = -1;
  if (mode == 2) {
    filter_id = getIntInput("Enter Product ID: ", 1, 999999);
  }

  printf("\n==================================================================="
         "==============================\n");
  printf("%-8s | %-19s | %-20s | %-6s | %-6s | %-10s | %-10s\n", "TXN ID",
         "Timestamp", "Product Name", "Type", "Qty", "Unit Price", "Total ($)");
  printf("---------------------------------------------------------------------"
         "----------------------------\n");

  int shown = 0;
  for (int i = 0; i < transaction_count; i++) {
    if (filter_id != -1 && sales_log[i].product_id != filter_id) {
      continue;
    }

    int idx = findProductIndex(sales_log[i].product_id);
    char prod_name[MAX_NAME_LEN];
    if (idx != -1) {
      strncpy(prod_name, inventory[idx].name, MAX_NAME_LEN);
    } else {
      snprintf(prod_name, MAX_NAME_LEN, "ID:%d (Deleted)",
               sales_log[i].product_id);
    }

    const char *type_str = (sales_log[i].type == 'S') ? "SALE" : "PURCH";

    printf("%-8d | %-19s | %-20.20s | %-6s | %6d | %10.2f | %10.2f\n",
           sales_log[i].transaction_id, sales_log[i].date_time, prod_name,
           type_str, sales_log[i].quantity, sales_log[i].unit_price,
           sales_log[i].total_amount);
    shown++;
  }

  if (shown == 0) {
    printf(" No transaction records found.\n");
  }
  printf("====================================================================="
         "============================\n");
}

/**
 * calculateInventoryValue - Aggregates total retail valuation of current stock.
 */
void calculateInventoryValue(void) {
  double total_retail = 0.0;
  int total_units = 0;

  for (int i = 0; i < product_count; i++) {
    total_retail += (inventory[i].price * inventory[i].quantity);
    total_units += inventory[i].quantity;
  }

  printf("====================================================\n");
  printf("             INVENTORY VALUATION SUMMARY            \n");
  printf("====================================================\n");
  printf(" Unique Product Types:  %d\n", product_count);
  printf(" Total Physical Units:  %d\n", total_units);
  printf(" Total Valuation (USD): $%.2f\n", total_retail);
  printf("====================================================\n");
}

/**
 * generateDailyReport - Filters transaction records matching current system
 * date.
 */
void generateDailyReport(void) {
  char today[20];
  getCurrentDate(today, sizeof(today));

  printf("====================================================\n");
  printf("        DAILY ANALYTICS REPORT: %s         \n", today);
  printf("====================================================\n");

  double total_revenue = 0.0;
  double total_procurement = 0.0;
  int units_sold = 0;
  int units_procured = 0;
  int sale_txns = 0;

  for (int i = 0; i < transaction_count; i++) {
    if (strncmp(sales_log[i].date_time, today, 10) == 0) {
      if (sales_log[i].type == 'S') {
        total_revenue += sales_log[i].total_amount;
        units_sold += sales_log[i].quantity;
        sale_txns++;
      } else if (sales_log[i].type == 'P') {
        total_procurement += sales_log[i].total_amount;
        units_procured += sales_log[i].quantity;
      }
    }
  }

  printf(" Sales Completed:       %d transactions\n", sale_txns);
  printf(" Items Sold Today:      %d units\n", units_sold);
  printf(" Gross Sales Revenue:   $%.2f\n", total_revenue);
  printf("----------------------------------------------------\n");
  printf(" Units Procured (Restock): %d units\n", units_procured);
  printf(" Procurement Expense:      $%.2f\n", total_procurement);
  printf("----------------------------------------------------\n");
  printf(" Net Daily Cash Flow:      $%.2f\n",
         total_revenue - total_procurement);
  printf("====================================================\n");
}
