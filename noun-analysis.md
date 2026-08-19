# Noun–Verb Analysis — College Course Registration System

Based on: Use Case 1 (Register for Course), Use Case 2 (Make Semester Fee Payment), Use Case 3 (Approve Registration)

## The Four Filters Applied
1. **Redundant** — eliminate if it's a duplicate/synonym of another candidate already listed
2. **Vague** — eliminate if too ambiguous or broad to represent a concrete class
3. **Attribute** — eliminate if it's really just a property/value belonging to another class, not a class itself
4. **Operation** — eliminate if it's really an action/behavior (a verb in noun form), not a thing

## Raw Candidate List (all nouns extracted from the three specs)
Student, Faculty/Advisor, Advisor, Faculty/HOD, College Administration, College Accountant, Payment Gateway, Course, Account, Registration period, Registration hold, Course Registration, Registration, Seat, Seat count, Registration status, System, Eligibility, Semester fee amount, Payment, Payment status, Payment confirmation, Payment method, Payment details, Approval, Rejection, Reason, Backlog registration

## Filtering Table

| Candidate | Filter Applied | Verdict | Reason |
|---|---|---|---|
| Student | — | **Survivor** | Concrete actor with attributes and behavior (registers, pays, views status) |
| Faculty/Advisor | Redundant | Eliminated | Duplicate of "Advisor" (same role, different naming in different specs) |
| Advisor | — | **Survivor** | Concrete actor with clear responsibilities (approve/reject registration) |
| Faculty/HOD | Redundant | Eliminated | Same role as Advisor in this context (reviews/approves special cases) |
| College Administration | Vague | Eliminated | Only a passive stakeholder ("needs records"); doesn't perform any action in the flows |
| College Accountant | — | **Survivor** | Concrete actor with a clear responsibility (verifying payment records) |
| Payment Gateway | — | **Survivor** | External system actor that performs a real function (processes transactions) |
| Course | — | **Survivor** | Concrete class with its own data (seats, eligibility rules) |
| Account | Attribute | Eliminated | "Student has a valid account" — this is a status/property of Student, not its own class |
| Registration period | Attribute | Eliminated | A time-bound setting/property checked during registration, not a class |
| Registration hold | Attribute | Eliminated | A status flag on the Student's record, not a class |
| Course Registration | Redundant | Eliminated | Duplicate of "Registration" |
| Registration | — | **Survivor** | Central class connecting Student and Course, with its own status and lifecycle |
| Seat | Attribute | Eliminated | Represents a count/availability value belonging to Course |
| Seat count | Attribute | Eliminated | Same as above — an attribute of Course |
| Registration status | Attribute | Eliminated | A property/state of the Registration class |
| System | Vague | Eliminated | Refers to the software as a whole, not a specific domain class |
| Eligibility | Vague | Eliminated | A rule/condition checked, not a concrete "thing" |
| Semester fee amount | Attribute | Eliminated | A value belonging to Payment |
| Payment | — | **Survivor** | Concrete class with its own data (amount, method, status, confirmation) |
| Payment status | Attribute | Eliminated | A property of the Payment class |
| Payment confirmation | Attribute | Eliminated | A derived output/property of Payment |
| Payment method | Attribute | Eliminated | A property of Payment (e.g., card, UPI, net banking) |
| Payment details | Vague | Eliminated | Too broad — the actual details are already captured as Payment's attributes |
| Approval | Operation | Eliminated | This is really the action `approve()` performed on a Registration, not a class |
| Rejection | Operation | Eliminated | This is really the action `reject()` performed on a Registration, not a class |
| Reason | Attribute | Eliminated | A property of a rejected Registration (rejection reason) |
| Backlog registration | Redundant | Eliminated | A type/category of Registration, not a separate class (handled via an attribute like `isBacklog`) |

## Surviving Classes

| Class | Responsibility (one line) |
|---|---|
| **Student** | Requests registration, makes payment, views status |
| **Advisor** | Reviews and approves/rejects a student's registration |
| **Accountant** | Verifies payment records |
| **PaymentGateway** | Processes the electronic payment transaction |
| **Course** | Holds course info, seat availability, eligibility rules |
| **Registration** | Links a Student to Course(s); tracks status (pending/approved/rejected), backlog flag |
| **Payment** | Records amount, method, status, and confirmation of a transaction |

These seven classes form the basis for the next steps (CRC cards and the domain class diagram).
