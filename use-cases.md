# Use Case Specifications — College Course Registration System

## Use Case 1: Register for Course

**Primary Actor:** Student

**Stakeholders:**
- Student — wants to register for required courses.
- Faculty/Advisor — wants students to register for eligible courses.
- College Administration — needs accurate registration records.

**Preconditions:**
- Student has a valid account.
- Registration period is open.
- Student has no registration hold.

**Postconditions:**
- Selected course registration is recorded.
- Available seat count is updated.
- Student can view the updated registration status.

**Trigger:** The student chooses the "Register for Course" option.

**Main Flow:**
1. Student logs into the system.
2. Student views available semester courses.
3. Student selects a course.
4. System checks eligibility and course availability.
5. System checks whether a seat is available.
6. Student confirms registration.
7. System records the registration.
8. System updates seat availability.
9. System displays registration confirmation.

**Alternate Flows:**
- **A1 — Course is full:** If no seat is available, the system informs the student and does not complete registration.
- **A2 — Student is not eligible:** The system displays an eligibility message and prevents registration.
- **A3 — Registration period closed:** The system informs the student that registration cannot be completed.

---

## Use Case 2: Make Semester Fee Payment

**Primary Actor:** Student

**Stakeholders:**
- Student — wants to complete the required payment.
- College Accountant — needs accurate payment records.
- Payment Gateway — processes the electronic transaction.

**Preconditions:**
- Student is logged into the system.
- Semester fee amount has been generated.
- Payment service is available.

**Postconditions:**
- Successful payment is recorded.
- Payment status is updated.
- Payment confirmation is available to the student.

**Trigger:** The student selects the "Make Payment" option.

**Main Flow:**
1. Student opens the semester fee payment page.
2. System displays the amount due.
3. Student selects a payment method.
4. Student provides the required payment details through the payment gateway.
5. Payment Gateway processes the transaction.
6. Payment Gateway returns a successful response.
7. System records the payment.
8. System updates the payment status.
9. System displays payment confirmation.

**Alternate Flows:**
- **A1 — Payment fails:** The system displays a failure message and does not mark the fee as paid.
- **A2 — Payment cancelled:** The system returns to the payment page without recording a successful payment.
- **A3 — Payment service unavailable:** The system informs the student to try again later.

---

## Use Case 3: Approve Registration

**Primary Actor:** Advisor

**Stakeholders:**
- Advisor — checks whether the student's registration is appropriate.
- Student — wants the selected courses approved.
- Faculty/HOD — may review special course or backlog registrations.
- College Administration — needs approved registration records.

**Preconditions:**
- Student has submitted course registration.
- Advisor is authenticated.
- Registration is awaiting approval.

**Postconditions:**
- Registration is marked approved or rejected.
- Student is notified of the decision.
- Final registration status is stored.

**Trigger:** The advisor opens a student's pending registration request.

**Main Flow:**
1. Advisor logs into the registration system.
2. System displays pending registration requests.
3. Advisor selects a student's registration.
4. System displays selected courses and registration details.
5. Advisor checks the registration.
6. Advisor approves the registration.
7. System changes the status to approved.
8. System stores the approval.
9. System notifies the student.

**Alternate Flows:**
- **A1 — Registration rejected:** Advisor rejects the registration and provides a reason. The system records the rejection and notifies the student.
- **A2 — Additional review required:** If HOD/Faculty approval is required, the system forwards the request for additional approval.
- **A3 — Incomplete details:** If required information is missing, the system asks the student to correct the registration before approval.
