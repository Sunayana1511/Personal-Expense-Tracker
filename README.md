# Personal Expense Tracker 💰

## 📌 About the Project

The **Personal Expense Tracker** is a console-based financial management application developed using C++17 and Object-Oriented Programming (OOP). It is designed primarily for college students and individuals who want a simple and organized way to monitor their daily income, expenses, and monthly budgets.

The application helps users maintain financial records, track spending habits, analyze monthly expenses, and manage their available balance through an interactive menu-driven interface.

## 🎯 Objectives

- Encourage better personal financial management.
- Help students monitor daily spending and income.
- Maintain organized transaction records.
- Set monthly spending limits and monitor budgets.
- Provide a clear overview of financial activity.

## ✨ Key Features

- **Expense Management:** Add, view, and delete expense records.
- **Income Tracking:** Record money received from different sources.
- **Category-Based Tracking:** Organize expenses into Food, Travel, Shopping, Rent, Entertainment, Health, and other categories.
- **Monthly Budget:** Set a spending limit and monitor budget usage.
- **Financial Summary:** View monthly income, expenses, and remaining balance.
- **Spending Analysis:** Review expenses by category and identify major spending areas.
- **Data Persistence:** Save and retrieve transaction records using CSV file handling.
- **Automatic Saving:** Preserve financial records when exiting the application.

## 🛠️ Technologies Used

- **Programming Language:** C++17
- **Concepts:** Object-Oriented Programming
- **Data Structures:** Vectors, Maps, Smart Pointers
- **File Handling:** CSV and File I/O
- **Libraries:** STL, iostream, fstream, vector, map, memory, algorithm

## 🧩 OOP Concepts Implemented

| Concept | Implementation |
|---|---|
| Encapsulation | Private data members in classes |
| Abstraction | Abstract Transaction class |
| Inheritance | Expense and Income inherit from Transaction |
| Polymorphism | Virtual functions and smart pointers |
| Operator Overloading | Date comparison and stream operators |

Additional concepts include RAII, STL containers, and the Factory Method for reconstructing transactions from CSV records.

## 👥 Target Users

- College and university students
- Individuals managing personal finances
- Beginners learning C++ and OOP
- Users looking for a simple budgeting solution

## ⚙️ How to Run

**1. Clone the repository**

```bash
git clone https://github.com/YOUR-USERNAME/Personal-Expense-Tracker.git
```

**2. Navigate to the project folder**

```bash
cd Personal-Expense-Tracker
```

**3. Compile the program**

```bash
g++ -std=c++17 -Wall -Wextra -o expense_tracker expense_tracker.cpp
```

**4. Run the application**

```bash
./expense_tracker
```

## 💾 Data Storage

The application stores transaction details and budget information in a CSV file named `tracker_data.csv`. Records are loaded when the application starts and saved automatically when changes are made.

## 🚀 Future Scope

- Graphical user interface (GUI)
- Visual expense analytics and charts
- Exportable monthly financial reports
- Additional financial planning tools
- Enhanced data security

## 👩‍💻 Author

**Sunayana Sinha**

BCA (Hons.) Data Science

Personal Expense Tracker | C++ OOP Project

---

*Developed as an academic project to demonstrate practical implementation of Object-Oriented Programming and file handling in C++.*
