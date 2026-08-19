# CRC Cards — College Course Registration System

One CRC (Class–Responsibility–Collaborator) card per surviving class from the noun–verb analysis.

## Student

| Class | Responsibilities | Collaborators |
|---|---|---|
| **Student** | - Log into the system<br>- View available semester courses<br>- Select and register for a course<br>- Register backlog course(s)<br>- Initiate semester fee payment<br>- Provide payment details<br>- View registration status<br>- View payment confirmation | Registration, Course, Payment |

## Advisor

| Class | Responsibilities | Collaborators |
|---|---|---|
| **Advisor** | - Log into the registration system<br>- View pending registration requests<br>- Check a student's registration details<br>- Approve a registration<br>- Reject a registration and provide a reason<br>- Forward request for additional (HOD) review if required | Registration, Student |

## Accountant

| Class | Responsibilities | Collaborators |
|---|---|---|
| **Accountant** | - Verify a student's payment record<br>- Confirm accuracy of payment before registration is unlocked | Payment, Student |

## PaymentGateway

| Class | Responsibilities | Collaborators |
|---|---|---|
| **PaymentGateway** | - Process the electronic payment transaction<br>- Return success/failure response to the system<br>- Handle cancelled or unavailable payment service scenarios | Payment, Student |

## Course

| Class | Responsibilities | Collaborators |
|---|---|---|
| **Course** | - Store course details (name, department, eligibility rules)<br>- Track available seat count<br>- Check and update seat availability upon registration | Registration |

## Registration

| Class | Responsibilities | Collaborators |
|---|---|---|
| **Registration** | - Link a Student to a Course<br>- Track registration status (pending/approved/rejected)<br>- Track backlog flag (regular vs. backlog course)<br>- Record approval or rejection details<br>- Notify student of status changes | Student, Course, Advisor |

## Payment

| Class | Responsibilities | Collaborators |
|---|---|---|
| **Payment** | - Store fee amount due<br>- Record payment method<br>- Track payment status (pending/successful/failed)<br>- Generate payment confirmation<br>- Provide verified payment record to Accountant | Student, Accountant, PaymentGateway |
