#ifndef __RANDOMMIXIN_H__
#define __RANDOMMIXIN_H__

#include <shogun/base/SGObject.h>
#include <shogun/lib/config.h>

#include <random>
#include <iterator>
#include <type_traits>

namespace shogun
{
	static inline void seed_callback(CSGObject*, int32_t);
	static inline void random_seed_callback(CSGObject*);

	namespace _seed
	{
		static constexpr auto seed_key = "seed";
		static constexpr auto random_seed_key = "set_random_seed";
	}

	template <typename Parent, typename PRNG = std::mt19937_64>
	class RandomMixin : public Parent
	{
	private:
		using this_t = RandomMixin<Parent, PRNG>;

	public:
		using prng_type = PRNG;

		template <typename... T>
		RandomMixin(T... args) : Parent(args...)
		{
			init();
		}

		virtual CSGObject* clone() const override
		{
			auto clone = dynamic_cast<this_t*>(Parent::clone());
			clone->m_prng = m_prng;
			return clone;
		}

	private:
		void init_random_seed()
		{
			typename PRNG::result_type random_data[PRNG::state_size];
			std::random_device source;
			std::generate(std::begin(random_data), std::end(random_data), std::ref(source));
			std::seed_seq seeds(std::begin(random_data), std::end(random_data));
			m_prng = PRNG(seeds);
		}

		bool set_random_seed()
		{
			init_random_seed();
			random_seed_callback(this);
			return true;
		}

		void init()
		{
			init_random_seed();

			Parent::watch_param(_seed::seed_key, &m_seed);
			Parent::add_callback_function(_seed::seed_key, [&]() {
				m_prng = PRNG(m_seed);
				seed_callback(this, m_seed);
			});

			Parent::watch_method(_seed::random_seed_key, &this_t::set_random_seed);
		}

		int32_t m_seed;

	protected:
		PRNG m_prng;
	};

	static inline void seed_callback(CSGObject* obj, int32_t seed)
	{
		obj->for_each_param_of_type<CSGObject*>(
		    [&](const std::string& name, CSGObject** param) {
			    if ((*param)->has(_seed::seed_key))
				    (*param)->put(_seed::seed_key, seed);
			    else
				    seed_callback(*param, seed);
		    });
	}

	static inline void random_seed_callback(CSGObject* obj)
	{
		obj->for_each_param_of_type<CSGObject*>(
		    [&](const std::string& name, CSGObject** param) {
			    if ((*param)->has(_seed::seed_key))
				    (*param)->run(_seed::random_seed_key);
			    else
				    random_seed_callback(*param);
		    });
	}
} // namespace shogun

#endif // __RANDOMMIXIN_H__
