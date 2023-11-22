#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>

// Constant value for BMR (Basal Metabolic Rate)
#define BMR 7500

int main() {
    // Seed the random number generator with the current time
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // Total weight change needed for 3 meals
    double totalWeightChangeNeeded = 0.0;

    for (int meal = 1; meal <= 3; ++meal) {
        // Hardcoded food options for testing
        std::string foodOption;
        if (meal == 1) {
            foodOption = "chicken breast";
        } else if (meal == 2) {
            foodOption = "salmon";
        } else if (meal == 3) {
            foodOption = "french fries";
        }

        // Convert user input to lowercase for comparison
        for (char& c : foodOption) {
            c = std::tolower(c);
        }

        // Generate a random weight between 0 and 500 grams
        double weightInGrams = std::rand() % 501;

        // Calories per gram for each hardcoded food option
        double caloriesPerGram = 0.0;

        if (foodOption == "chicken breast") {
            caloriesPerGram = 1.965;
        } else if (foodOption == "salmon") {
            caloriesPerGram = 2.059;
        } else if (foodOption == "french fries") {
            caloriesPerGram = 1.461;
        } else {
            // Handle unknown food option
            std::cout << "Unknown food option: " << foodOption << std::endl;
            continue;
        }

        // Calculate total calories based on weight and calories per gram
        double totalCalories = weightInGrams * caloriesPerGram;

        // Calculate weight change needed
        double weightChangeNeeded = ((BMR / 3.0) - totalCalories) * (1.0 / caloriesPerGram);

        // Accumulate weight change needed for 3 meals
        totalWeightChangeNeeded += weightChangeNeeded;

        // Print information for each meal
        std::cout << "Meal " << meal << ":\n";
        if (totalCalories > BMR) {
            std::cout << "Calories consumed (" << totalCalories << ") exceed BMR (" << BMR << ").\n";
        } else {
            std::cout << "Calories consumed (" << totalCalories << ") are within BMR (" << BMR << ").\n";
        }
        std::cout << "Weight change needed: " << weightChangeNeeded << " grams\n";
    }

    // Print the total weight change needed for 3 meals
    std::cout << "Total weight change needed for 3 meals: " << totalWeightChangeNeeded << " grams\n";

    // Check if the sum of weight changes equals the user's BMR
    if (totalWeightChangeNeeded == BMR) {
        std::cout << "Test passed: Sum of weight changes equals the user's BMR.\n";
    } else {
        std::cout << "Test failed: Sum of weight changes does not equal the user's BMR.\n";
    }

    return 0;
}
