#pragma once

#include <vector>

// I am deeply sorry for why this is even a thing, but it seems to be a necessary evil

namespace InputFormTypes {
	enum InputFormTypes {
		MULTIPLE,
		INPUT,
		CONSTANT
	};
}

typedef size_t InputFormType;

struct InputForm {
public:
	

	InputFormType type;
	std::vector<InputForm>* otherForms = nullptr;



	InputForm(InputFormType type) {
		this->type = type;
		if (type == InputFormTypes::MULTIPLE) otherForms = new std::vector<InputForm>();
	}

	void add(InputForm i) {
		if (otherForms) otherForms->push_back(i);
	}

};