#pragma once

#include <gui/helpers.h>
#include <gui/controls/checkbox.h>
#include <gui/controls/combo_box.h>
#include <gui/controls/label.h>
#include <gui/controls/selectable.h>

namespace menu::ui
{
	using namespace evo::gui;
	using namespace evo::gui::helpers;
	using namespace evo::ren;

	using combo_option = std::pair<std::string, int>;

	inline std::shared_ptr<combo_box> combo(
		const std::string &id, value_param<bits> &value, const std::initializer_list<combo_option> &options = {}, bool multiple = false)
	{
		const auto c = std::make_shared<combo_box>(control_id{hash(id), id}, value);
		c->legacy_mode = !multiple;
		c->allow_multiple = multiple;

		for (const auto &[name, option_value] : options)
		{
			const auto option_id = id + XOR(">") + name;
			c->add(std::make_shared<selectable>(control_id{hash(option_id), option_id}, name, option_value));
		}

		return c;
	}

	inline std::shared_ptr<layout> row(
		const std::shared_ptr<layout> &e, const std::string &id, const std::string &text, const std::vector<std::shared_ptr<control>> &controls,
		const color &text_color = colors.text, bool disabled = false, bool marked = false)
	{
		const auto result = make_inlined(
			hash(id + XOR(">row")),
			[id, text, controls, text_color, disabled, marked](std::shared_ptr<layout> &row_layout)
			{
				const auto label_id = id + XOR(">label");
				const auto l = std::make_shared<label>(control_id{hash(label_id), label_id}, text, text_color);
				l->margin = {};
				l->is_new = marked;
				l->is_taking_input = !disabled;
				row_layout->add(l);

				// controls are written in visual left-to-right order, but s_inline stacks
				// later children right-to-left (first added = rightmost), so add them reversed.
				for (auto it = controls.rbegin(); it != controls.rend(); ++it)
				{
					(*it)->is_taking_input = !disabled;
					row_layout->add(*it);
				}
			});

		result->is_taking_input = !disabled;
		e->add(result);
		return result;
	}

	inline void add_combo(
		const std::shared_ptr<layout> &e, const std::string &id, const std::string &text, value_param<bits> &value,
		const std::initializer_list<combo_option> &options = {}, bool multiple = false)
	{
		row(e, id, text, {combo(id, value, options, multiple)});
	}

	inline void warning_row(
		const std::shared_ptr<layout> &e, const std::string &id, const std::string &text,
		const std::vector<std::shared_ptr<control>> &controls = {})
	{
		row(e, id, text, controls, colors.warning);
	}

	inline void disabled_row(const std::shared_ptr<layout> &e, const std::string &id, const std::string &text, const std::vector<std::shared_ptr<control>> &controls)
	{
		row(e, id, text, controls, colors.text_mid, true);
	}

	// "untrusted" feature gate (Misc > Extra > Untrusted features).
	// doubletap, hide shot and silent aim can only be used while that switch is on:
	// otherwise their rows stop taking input and the orange label gets slightly darkened.
	inline bool untrusted_features{};

	struct untrusted_gate_t
	{
		std::weak_ptr<layout> row;
		std::weak_ptr<checkbox> box;
	};

	inline std::vector<untrusted_gate_t> untrusted_gates{};

	inline void apply_untrusted_gates()
	{
		for (auto it = untrusted_gates.begin(); it != untrusted_gates.end();)
		{
			const auto r = it->row.lock();
			const auto b = it->box.lock();
			if (!r || !b)
			{
				it = untrusted_gates.erase(it);
				continue;
			}

			r->is_taking_input = untrusted_features;
			r->for_each_control(
				[&](std::shared_ptr<control> &c)
				{
					c->is_taking_input = untrusted_features;

					if (c->type == ctrl_label)
						c->as<label>()->col = untrusted_features ? colors.warning : colors.warning.darken(.25f);
				});

			// the feature can not stay on while the switch is off
			if (!untrusted_features && b->value)
				b->set_value(false);

			++it;
		}
	}

	inline std::shared_ptr<layout> untrusted_row(
		const std::shared_ptr<layout> &e, const std::string &id, const std::string &text,
		const std::vector<std::shared_ptr<control>> &controls, const std::shared_ptr<checkbox> &box)
	{
		const auto result =
			row(e, id, text, controls, untrusted_features ? colors.warning : colors.warning.darken(.25f), !untrusted_features);
		untrusted_gates.push_back({result, box});

		if (!untrusted_features && box->value)
			box->set_value(false);

		return result;
	}

	inline void set_untrusted_features(bool v)
	{
		untrusted_features = v;
		apply_untrusted_gates();
	}
} // namespace menu::ui
