#include "sppam.h"
uint64_t access_counter = 0;
uint32_t sppam::prefetcher_cache_operate(champsim::address addr, champsim::address ip, uint8_t cache_hit, bool useful_prefetch, access_type type,
                                         uint32_t metadata_in)
{
  // assert(addr == ip); // Invariant for instruction prefetchers
  uint64_t curr_offset = (addr >> log2(64)) & (0x3F); // 3F--63 --blovk offset from 0 to 63
  uint64_t page = addr >> 12;                          // 12 bits are offset
  int16_t delta;                                       // int --can be -ve too
  bool hit_status = false;
  uint16_t old_sig =0;
  bool lru_entry_hit = false;
  uint32_t replacement_entry;
  bool invalid_entry_hit = false;
  uint64_t final_hit_index=0;

  for (int i = 0; i < 255; i++) {
    if (page == s_table.Sig_table[i].tag && s_table.Sig_table[i].valid == true) // hit
    {
      delta = curr_offset - s_table.Sig_table[i].last_offset;
      old_sig = s_table.Sig_table[i].sig;
      s_table.Sig_table[i].sig = (s_table.Sig_table[i].sig << 3) ^ (delta);
      s_table.Sig_table[i].last_offset = delta;
      s_table.Sig_table[i].lru_last_used = access_counter;
      s_table.Sig_table[i].valid = true;
      hit_status = true;
      final_hit_index=i;
    }
  }
  uint64_t lru_min_counter = UINT64_MAX;
  if (!hit_status) // miss
  {
    for (int j = 0; j < 255; j++) {
      if (s_table.Sig_table[j].valid == false) {
        replacement_entry = j;
        invalid_entry_hit = true;
        break;
      }
    }

    if (invalid_entry_hit == false) // Ssearch for lru entry
    {
      for (int j = 0; j < 255; j++) {
        if (s_table.Sig_table[j].lru_last_used < lru_min_counter) {
          lru_min_counter = s_table.Sig_table[j].lru_last_used;
          replacement_entry=j;
          lru_entry_hit=true;
        }
      }
    }

    //add or replace the entry
      old_sig=0;
      delta=0;
      s_table.Sig_table[replacement_entry].tag = page;
      s_table.Sig_table[replacement_entry].sig = 0;
      s_table.Sig_table[replacement_entry].last_offset = curr_offset;
      s_table.Sig_table[replacement_entry].lru_last_used = access_counter;
      s_table.Sig_table[replacement_entry].valid = true;
  
    final_hit_index=replacement_entry;
  }
  update_pt(old_sig, delta);

  uint16_t curr_sig =s_table.Sig_table[final_hit_index].sig;
  prefetch_gen (); // to be implemented

  return metadata_in;
}

void sppam::update_pt(uint16_t sig, int8_t delta)
{

  /// To be implemented: if either c_delta or c_sig saturates, right-shift all counters in that entry by 1 instead of clamping
  uint16_t index;
  index = sig & 0x1FF;
  bool c_delta_match=false;
  uint16_t index_low_delta;
  uint16_t lowest_delta_value = 16;
  //inc c_sig
  p_table.pattern[index].c_sig = (p_table.pattern[index].c_sig>14) ? 15 : (p_table.pattern[index].c_sig + 1);   
  //inc c_delta
  for (int i=0;i<3;i++)
  {
    if(p_table.pattern[index].predictors[i].c_delta==delta)//match
    {
      p_table.pattern[index].predictors[i].c_delta = (p_table.pattern[index].predictors[i].c_delta>14) ? 15 : (p_table.pattern[index].predictors[i].c_delta + 1); 
      c_delta_match=true;
    }
    if(p_table.pattern[index].predictors[i].c_delta<lowest_delta_value)
    {
      index_low_delta=i;
    }

  }
  if (c_delta_match==false) //replacing the lowest delta entry
  {
    p_table.pattern[index].predictors[index_low_delta].c_delta = 1;
    
    p_table.pattern[index].predictors[index_low_delta].delta = delta;
  }
}


uint32_t sppam::prefetcher_cache_fill(champsim::address addr, long set, long way, uint8_t prefetch, champsim::address evicted_addr, uint32_t metadata_in)
{
  return metadata_in;
}
