// ==========================================================================
// Dedmonwakeen's Raid DPS/TPS Simulator.
// Send questions to natehieter@gmail.com
// ==========================================================================

#include "simulationcraft.hpp"

// set_bonus_t::decode ======================================================

set_e set_bonus_t::decode( const player_t& p,
                         const item_t&   item ) const
{
  if ( ! item.name() ) return SET_NONE;

  if ( p.sim -> challenge_mode ) return SET_NONE;

  set_e set = p.decode_set( item );
  if ( set != SET_NONE ) return set;

  return SET_NONE;
}

// set_bonus_t::init ========================================================

void set_bonus_t::init( const player_t& p )
{
  for ( size_t i = 0; i < p.items.size(); i++ )
  {
    set_e s = decode( p, p.items[ i ] );
    if ( s != SET_NONE )
    {
      count[ s ] += 1;
    }
  }
  initialized = true;
}

expr_t* set_bonus_t::create_expression( const player_t* player,
                                        const std::string& type )
{
  set_e bonus_type = util::parse_set_bonus( type );

  if ( bonus_type == SET_NONE )
    return 0;

  struct set_bonus_expr_t : public expr_t
  {
    const player_t& player;
    set_e set_bonus_type;
    set_bonus_expr_t( const player_t& p, set_e t ) :
      expr_t( util::set_bonus_string( t ) ), player( p ), set_bonus_type( t ) {}
    virtual double evaluate()
    { return player.sets.has_set_bonus( set_bonus_type ); }
  };

  assert( player );
  return new set_bonus_expr_t( *player, bonus_type );
}

inline const spell_data_t* set_bonus_t::create_set_bonus( uint32_t spell_id )
{
  if ( ! p -> dbc.spell( spell_id ) )
  {
    if ( spell_id > 0 )
    {
      p -> sim -> errorf( "Set bonus spell identifier %u for %s not found in spell data.",
                          spell_id, p -> name_str.c_str() );
    }
    return default_value;
  }

  return ( p -> dbc.spell( spell_id ) );
}

// set_bonus_array_t::set_bonus_array_t =====================================

set_bonus_t::set_bonus_t( const player_t* p ) :
     count(), default_value( spell_data_t::nil() ), set_bonuses(),  p( p ),
     initialized( false ), spelldata_registered( false )
{

    count[ SET_T13_2PC_CASTER ] = count[ SET_T13_2PC_MELEE ] = count[ SET_T13_2PC_TANK ] = count[ SET_T13_2PC_HEAL ] = -1;
    count[ SET_T13_4PC_CASTER ] = count[ SET_T13_4PC_MELEE ] = count[ SET_T13_4PC_TANK ] = count[ SET_T13_4PC_HEAL ] = -1;
    count[ SET_T14_2PC_CASTER ] = count[ SET_T14_2PC_MELEE ] = count[ SET_T14_2PC_TANK ] = count[ SET_T14_2PC_HEAL ] = -1;
    count[ SET_T14_4PC_CASTER ] = count[ SET_T14_4PC_MELEE ] = count[ SET_T14_4PC_TANK ] = count[ SET_T14_4PC_HEAL ] = -1;
    count[ SET_T15_2PC_CASTER ] = count[ SET_T15_2PC_MELEE ] = count[ SET_T15_2PC_TANK ] = count[ SET_T15_2PC_HEAL ] = -1;
    count[ SET_T15_4PC_CASTER ] = count[ SET_T15_4PC_MELEE ] = count[ SET_T15_4PC_TANK ] = count[ SET_T15_4PC_HEAL ] = -1;
    count[ SET_T16_2PC_CASTER ] = count[ SET_T16_2PC_MELEE ] = count[ SET_T16_2PC_TANK ] = count[ SET_T16_2PC_HEAL ] = -1;
    count[ SET_T16_4PC_CASTER ] = count[ SET_T16_4PC_MELEE ] = count[ SET_T16_4PC_TANK ] = count[ SET_T16_4PC_HEAL ] = -1;
    count[ SET_T17_2PC_CASTER ] = count[ SET_T17_2PC_MELEE ] = count[ SET_T17_2PC_TANK ] = count[ SET_T17_2PC_HEAL ] = -1;
    count[ SET_T17_4PC_CASTER ] = count[ SET_T17_4PC_MELEE ] = count[ SET_T17_4PC_TANK ] = count[ SET_T17_4PC_HEAL ] = -1;
    count[ SET_PVP_2PC_CASTER ] = count[ SET_PVP_2PC_MELEE ] = count[ SET_PVP_2PC_TANK ] = count[ SET_PVP_2PC_HEAL ] = -1;
    count[ SET_PVP_4PC_CASTER ] = count[ SET_PVP_4PC_MELEE ] = count[ SET_PVP_4PC_TANK ] = count[ SET_PVP_4PC_HEAL ] = -1;
}

void set_bonus_t::register_spelldata( const set_bonus_description_t a_bonus )
{
  // Map two-dimensional array into correct slots in the one-dimensional set_bonuses
  // array, based on set_e enum
  for ( int tier = 0; tier < N_TIER; ++tier )
  {
    for ( int j = 0; j < N_TIER_BONUS; j++ )
    {
      int b = 3 * j / 2 + 1;
      set_bonuses[ 1 + tier * 12 + b ] = create_set_bonus( a_bonus[ tier ][ j ] );
    }
  }
  spelldata_registered = true;
}

/* Check if the player has the set bonus
 */
bool set_bonus_t::has_set_bonus( set_e s ) const
{
  // Disable set bonuses in challenge mode
  if ( p -> sim -> challenge_mode == 1 )
    return false;

  return set_bonus_t::has_set_bonus( p, s );
}

bool set_bonus_t::has_set_bonus( const player_t* p, set_e s )
{
  assert( p -> sets.initialized && "Attempt to check for set bonus before initialization." );

  if ( p -> sets.count[ s ] > 0 )
    return true;

  if ( p -> sets.count[ s ] < 0 )
  {
    assert( s > 0 );
    int tier  = ( s - 1 ) / 12,
        btype = ( ( s - 1 ) % 12 ) / 3,
        bonus = ( s - 1 ) % 3;

    assert( 1 + tier * 12 + btype * 3 < as<int>( p -> sets.count.size() ) );
    if ( p -> sets.count[ 1 + tier * 12 + btype * 3 ] >= bonus * 2 )
      return true;
  }

  return false;
}

/* Return pointer to set_bonus spell_data
 * spell_data_t::nil() if player does not have set_bonus
 */
const spell_data_t* set_bonus_t::set( set_e s ) const
{
  assert( p -> sets.spelldata_registered && "Attempt to use set bonus spelldata before registering spelldata." );

  if ( set_bonuses[ s ] && has_set_bonus( s )  )
    return set_bonuses[ s ];

  return default_value;
}

void set_bonus_t::copy_from( const set_bonus_t& s )
{ count = s.count; }

namespace new_set_bonus {
set_bonus_t::set_bonus_t( const player_t* p ) :
  default_value( spell_data_t::nil() ),
  set_bonuses(),
  count(),
  p( p ),
  initialized( false ),
  spelldata_registered( false )
{
  // Fill up count array with default data.
  for ( size_t i = 0; i < set_bonuses.size(); ++i )
  {
    count[i].resize( p -> dbc.specialization_max_per_class() );
  }

  // Fill up set_bonus array for specs
  for ( size_t i = 0; i < set_bonuses.size(); ++i )
  {
    set_bonuses[i].resize( p -> dbc.specialization_max_per_class() );
  }
}

void set_bonus_t::init()
{
  // TODO: fill up count array

  // TODO: retrieve all set bonus data, and map it into set_bonus, depending on has_set_bonus

  initialized = true;
}

/* Retrieve spec index from given specialization_e enum.
 * Check if this is a performance bottle-neck or not.
 */
uint32_t set_bonus_t::get_spec_idx( specialization_e spec ) const
{
  uint32_t class_idx, spec_idx;
  if ( !p -> dbc.spec_idx( spec, class_idx, spec_idx ) )
    throw std::logic_error("Could not determine class/spec from specialization_e enum");

  assert( class_idx == as<uint32_t>(util::class_id_mask( p -> type )) && "Trying to get set bonus for spec of wrong class!" );
  return spec_idx;
}

const spell_data_t* set_bonus_t::set( set_tier_e tier, unsigned pieces, specialization_e spec ) const
{
  assert( initialized && "Attempt to check for set bonus before initialization." );

  const spell_data_t* s = default_value;
  uint32_t spec_idx = get_spec_idx( spec );

  assert( set_bonuses[ tier ].size() >= spec_idx ); // should be filled in ctor
  if ( set_bonuses[ tier ][ spec_idx ].size() >= pieces )
  {
    s = set_bonuses[ tier ][ spec_idx ][ pieces ];

  }
  assert( !( has_set_bonus( tier, pieces, spec ) && s == default_value ) && "Player has given set bonus, but no non-nil spell data for it initialized!" );

  return s;

}

bool set_bonus_t::has_set_bonus( set_tier_e tier, unsigned pieces, specialization_e spec ) const
{
  assert( initialized && "Attempt to check for set bonus before initialization." );

  uint32_t spec_idx = get_spec_idx( spec );

  assert( count[ tier ].size() >= spec_idx ); // should be filled in ctor
  if ( count[ tier ][ spec_idx ] >= pieces )
    return true;


  return false;
}

}

