#include "tmesh.h"
#include "util_sys.h"
#include "util_unix.h"
#include "unit_test.h"

#include "./tmesh_device.c"
extern struct radio_struct test_device;

int main(int argc, char **argv)
{
  fail_unless(!e3x_init(NULL)); // random seed
  fail_unless(radio_device(&test_device));
  
  mesh_t meshA = mesh_new(3);
  fail_unless(meshA);
  lob_t secretsA = mesh_generate(meshA);
  fail_unless(secretsA);

  lob_t idB = e3x_generate();
  hashname_t hnB = hashname_keys(lob_linked(idB));
  fail_unless(hnB);
  link_t link = link_get(meshA,hnB->hashname);
  fail_unless(link);
  
  tmesh_t netA = tmesh_new(meshA, NULL);
  fail_unless(netA);
  cmnty_t c = tmesh_private(netA,"fzjb5f4tn4","foo");
  fail_unless(c);
  fail_unless(c->medium->bin[0] == 46);
  fail_unless(c->pipe->path);
  LOG("netA %.*s",c->pipe->path->head_len,c->pipe->path->head);


  fail_unless(!tmesh_public(netA, "kzdhpa5n6r", NULL));
  fail_unless((c = tmesh_public(netA, "kzdhpa5n6r", "")));
  mote_t m = tmesh_link(netA, c, link);
  fail_unless(m);
  fail_unless(m->link == link);
  fail_unless(m->epochs);
  fail_unless(!m->knock);
  fail_unless(m->kstate == SKIP);
  fail_unless(m == tmesh_link(netA, c, link));

  fail_unless(mote_knock(m, c->medium, 1));
  fail_unless(m->knock);
  fail_unless(m->kstate == READY);
  LOG("%d %d %d",m->kstart,m->kstop,m->kchan);
  fail_unless(m->kstart);
  fail_unless(m->kstop == (m->kstart + 10));
  fail_unless(m->kchan < 100);
  uint8_t chan = m->kchan;
  m->kchan = 101; // set to bad value to make sure prep resets it

  fail_unless(radio_prep(&test_device, netA, 1));
  fail_unless(m == radio_get(&test_device, netA));
  fail_unless(m->kchan == chan);

  return 0;
}
